#!/usr/bin/env python3
"""Extract a fractional subset of an HDF5 file, preserving its exact structure."""

import argparse
import os
import h5py


def parse_args():
    parser = argparse.ArgumentParser(description="Extract a fractional subset of an HDF5 file, preserving its structure.")
    parser.add_argument("-f", "--file", required=True, help="Input HDF5 file")
    parser.add_argument("-r", "--ratio", type=float, required=True, help="Fraction of rows to keep (e.g. 0.02 for 2%%)")
    return parser.parse_args()


def main():
    args = parse_args()

    input_path = args.file
    ratio = args.ratio

    if not 0 < ratio <= 1:
        raise ValueError("ratio must be between 0 (exclusive) and 1 (inclusive)")

    # Derive output path by inserting ".small" before the extension
    base, ext = os.path.splitext(input_path)
    output_path = base + ".small" + ext

    with h5py.File(input_path, "r") as input_file, h5py.File(output_path, "w") as output_file:

        # HDFv2 files link /Events -> /EventIndices -> /FEEHits, so these must be
        # truncated consistently: keep K events, then keep exactly the /FEEHits
        # rows those events reference. (V1 files have no /EventIndices.)
        events_cutoff = None
        feehits_cutoff = None
        if "EventIndices" in input_file and "Events" in input_file:
            n_events = input_file["Events"].shape[0]
            events_cutoff = min(max(1, int(n_events * ratio)), n_events)
            if events_cutoff > 0:
                feehits_cutoff = int(input_file["EventIndices"][events_cutoff - 1]["fee_hits"][1])

        for name, dataset in input_file.items():
            if dataset.shape == (1,) or dataset.shape == ():
                # Scalar/config datasets are small — copy them in full using [()] to read all data
                output_dataset = output_file.create_dataset(name, data=dataset[()], dtype=dataset.dtype)
            else:
                # Pick the row count, keeping the relational V2 datasets consistent
                if name in ("Events", "EventIndices") and events_cutoff is not None:
                    number_of_rows = min(events_cutoff, dataset.shape[0])
                elif name == "FEEHits" and feehits_cutoff is not None:
                    number_of_rows = min(feehits_cutoff, dataset.shape[0])
                else:
                    # Generic: at least 1 row for tiny ratios, never more than the source has
                    number_of_rows = max(1, int(dataset.shape[0] * ratio))
                    number_of_rows = min(number_of_rows, dataset.shape[0])

                if number_of_rows == 0:
                    # An empty dataset cannot be chunked or compressed
                    output_dataset = output_file.create_dataset(name, data=dataset[:0], dtype=dataset.dtype)
                else:
                    # Cap chunk size to the output row count so it never exceeds the data shape
                    chunks = (min(dataset.chunks[0], number_of_rows),) if dataset.chunks else None
                    output_dataset = output_file.create_dataset(name, data=dataset[:number_of_rows], dtype=dataset.dtype,
                                                                compression=dataset.compression,
                                                                compression_opts=dataset.compression_opts,
                                                                chunks=chunks)
                print(f"  {name}: {dataset.shape[0]} -> {number_of_rows} rows")

            # Copy the dataset's attributes (e.g. HDFVersion, Config on /Events)
            for attr_name, attr_value in dataset.attrs.items():
                output_dataset.attrs[attr_name] = attr_value

        # Copy file-level (root group) attributes
        for attr_name, attr_value in input_file.attrs.items():
            output_file.attrs[attr_name] = attr_value

    print(f"Written {os.path.getsize(output_path) / 1024**2:.1f} MB to {output_path}")


if __name__ == "__main__":
    main()
