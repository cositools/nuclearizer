#!/usr/bin/env python3

import argparse
from pathlib import Path

import numpy as np
import h5py

def select_and_split(eventID, infname, outfname):
    """
        Select given events in the input HDF and create a new HDF
    """

    # Ensure eventID is sorted for clean indexing
    eventID = np.sort(eventID)

    with h5py.File(infname, 'r') as infile, h5py.File(outfname, 'w') as outfile:

        # 1. Filter and Process Events
        print("Processing Events...")
        events_data = infile['Events'][()][eventID]
        ds_events = outfile.create_dataset('Events', data=events_data)
        for attr_name, attr_value in infile['Events'].attrs.items():
            ds_events.attrs[attr_name] = attr_value

        # 2. Filter and Process EventIndices
        print("Processing EventIndices...")
        indices_data = infile['EventIndices'][()][eventID]

        # Calculate hit counts for each event
        fee_hit_counts = (indices_data['fee_hits'][:, 1] - indices_data['fee_hits'][:, 0]).astype(np.uint32)

        # Calculate new contiguous bounds
        new_starts = np.insert(np.cumsum(fee_hit_counts)[:-1], 0, 0)
        new_ends = new_starts + fee_hit_counts

        # Update the bounds in the EventIndices data
        indices_data['fee_hits'][:, 0] = new_starts
        indices_data['fee_hits'][:, 1] = new_ends

        ds_indices = outfile.create_dataset('EventIndices', data=indices_data)
        for attr_name, attr_value in infile['EventIndices'].attrs.items():
            ds_indices.attrs[attr_name] = attr_value

        # 3. Filter and Process FEEHits
        print("Processing FEEHits...")
        all_fee_hits = infile['FEEHits'][()] # Load once into memory

        # Use np.isin to get the mask, exactly as you originally wrote
        mask = np.isin(all_fee_hits['event_index'], eventID)
        selected_hits = all_fee_hits[mask]

        # Overwrite the event_index to map to the new row numbers (0 to N)
        new_event_indices = np.repeat(np.arange(len(eventID)), fee_hit_counts)
        selected_hits['event_index'] = new_event_indices

        ds_hits = outfile.create_dataset('FEEHits', data=selected_hits)
        for attr_name, attr_value in infile['FEEHits'].attrs.items():
            ds_hits.attrs[attr_name] = attr_value

        # 4. Copy all other tables directly
        for key in infile.keys():
            if key not in ['Events', 'EventIndices', 'FEEHits']:
                print(f"Copying {key} as is...")
                infile.copy(key, outfile)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Split GSE HDF5 file based on event IDs.")
    parser.add_argument("hdf_file", type=Path, help="Path to the input HDF5 file.")
    parser.add_argument("txt_file", type=Path, help="Path to the text file containing event IDs (one per line).")

    args = parser.parse_args()

    if not args.hdf_file.exists():
        raise FileNotFoundError(f"Input HDF5 file not found: {args.hdf_file}")
    if not args.txt_file.exists():
        raise FileNotFoundError(f"Text file not found: {args.txt_file}")

    print(f"Reading event IDs from {args.txt_file}...")
    # Load event IDs from the text file (assuming one ID per line)
    event_ids = np.loadtxt(args.txt_file, dtype=np.int64)

    # The output HDF5 should be stored in the same directory as the input HDF5
    # and have the same prefix (stem) as the text file.
    out_file = args.hdf_file.parent / f"{args.txt_file.stem}.hdf5"

    print(f"Splitting HDF5. Output will be saved to: {out_file}")
    select_and_split(event_ids, args.hdf_file, out_file)
    print("Done!")
