#!/usr/bin/env python3
"""Remove event IDs and change \"board\" entry in the Config JSON string to \"RTB\", otherwise preserving its exact structure."""

import argparse
import os
import h5py
import re


def parse_args():
    parser = argparse.ArgumentParser(description="Remove event IDs and change \"board\" entry in the Config JSON string to \"RTB\"")
    parser.add_argument("-f", "--file", required=True, help="Input HDF5 file")
    return parser.parse_args()


def main():
    args = parse_args()

    input_path = args.file
    # Derive output path by inserting ".rtb" before the extension
    base, ext = os.path.splitext(input_path)
    output_path = base + ".rtb" + ext

    with h5py.File(input_path, "r") as input_file, h5py.File(output_path, "w") as output_file:

        for name, dataset in input_file.items():

            # Read dataset content into a NumPy array
            data = dataset[...]

            # Set event IDs to zero for the "Events" dataset
            if name == "Events":
                first_field = data.dtype.names[0]
                data[first_field] = 0
            
            # Create dataset in output file
            output_dataset = output_file.create_dataset(name, data=data, dtype=dataset.dtype,
                                                    compression=dataset.compression,
                                                    compression_opts=dataset.compression_opts,
                                                    chunks=dataset.chunks)

            # Copy the dataset's attributes (e.g. HDFVersion, Config on /Events)
            for attr_name, attr_value in dataset.attrs.items():
                
                # change the "board" entry in the JSON Config string to RTB
                if name == "Events" and attr_name == "Config":
                    attr_value[0] = re.sub(r"dib", "RTB", attr_value[0], count=1, flags=re.IGNORECASE)
                    
                output_dataset.attrs[attr_name] = attr_value

        # Copy file-level (root group) attributes
        for attr_name, attr_value in input_file.attrs.items():
            output_file.attrs[attr_name] = attr_value

    print(f"Written file with \"board\":\"RTB\" and without event IDs to {output_path}")


if __name__ == "__main__":
    main()
