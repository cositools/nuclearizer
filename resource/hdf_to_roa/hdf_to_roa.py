from strip_map import StripMap
from asic_convert import AsicConvert
import os.path


def openfile(filename, mode="r"):
    if filename.endswith(".gz"):
        import gzip

        return gzip.open(filename, mode)
    else:
        return open(filename, mode)


def hdf_to_roa(
    hdf_file,
    stripmap_file,
    save_dir,
    save_name,
    verbose=False,
    gz=False,
    filter_side="both",
):
    print("Reading Strip Map")
    strip_map = StripMap(stripmap_file)

    print("Reading HDF File")
    if verbose:
        print(hdf_file)
    d = AsicConvert([hdf_file], verbose=verbose)
    asics = d.series.asic.max() + 1

    print("Writing ROA File")
    roa_save_dir = ("/").join(hdf_file.split("/")[:-1])
    if save_dir != None:
        roa_save_dir = save_dir.rstrip("/")
    roa_save_name = hdf_file.split("/")[-1]
    if save_name != None:
        roa_save_name = save_name
    print(roa_save_dir + "/" + roa_save_name)
    assert os.path.exists(roa_save_dir)

    if gz:
        opentype = "wt"
        filename = roa_save_dir + "/" + roa_save_name + ".roa.gz"
    else:
        opentype = "w"
        filename = roa_save_dir + "/" + roa_save_name + ".roa"

    with openfile(filename, opentype) as f:
        # TODO: group events by oscillator
        event_id = 0

        # TODO: more than 1 detector
        det = 0

        f.write("TYPE ROA\n")
        f.write("UF doublesidedstrip adc\n")

        # d.numpy is a numpy record array representation of the data
        # a record is (asic, channel, adc, tac, oscillator, headers) as defined in parse_asic.py
        prev_osc = -1
        offset = 0
        osc_freq = 200000.0
        for asic, ch, adc, tac, oscillator, headers, dt in d.numpy:
            (side, strip) = strip_map.get_side_strip(asic, ch)
            if side == 0:
                side = "p"
                if filter_side == "hv":
                    continue
            else:
                side = "n"
                if filter_side == "lv":
                    continue
            if oscillator != prev_osc:
                if oscillator < prev_osc:
                    offset += 65536
                f.write("\nSE\n")
                f.write("CL {:f}\n".format((offset + oscillator) / osc_freq))
                f.write("ID {}\n".format(event_id))
                event_id += 1
                prev_osc = oscillator
                f.write("UH {} {} {} {} {}\n".format(det, strip, side, adc, tac))


if __name__ == "__main__":
    from argparse import ArgumentParser

    p = ArgumentParser(
        description="temporary tool to convert hdf5 files to roa files that can be read in rrby nuclearizer"
    )
    p.add_argument("--hdf", default=None, nargs="?", help="full path to hdf5 filename")
    p.add_argument(
        "--save_dir",
        default=None,
        help="directory in which to save the roa file, defaults to hdf directory",
    )
    p.add_argument(
        "--save_name",
        default=None,
        help="file name for the roa file. Defaults to the same filename as the hdf file",
    )
    p.add_argument("--side", default="both")
    p.add_argument("--stripmap", help="path to stripmap file")
    p.add_argument("-v", "--verbose", action="store_true")
    p.add_argument("-gz", "--gz", default=False, action="store_true")
    args = p.parse_args()
    hdf_to_roa(
        args.hdf,
        args.stripmap,
        args.save_dir,
        args.save_name,
        args.verbose,
        args.gz,
        args.side,
    )
