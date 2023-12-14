import pandas
import numpy as np
import tables
from glob import glob
import pathlib
from typing import Iterable, Tuple


class Hdf:
    """Write and read acquired data in HDF format."""

    def __init__(self, filename: str, mode: str = 'w') -> None:
        """
        Create the filename for later opening.

        Filename specified is stored for later use when we open
        the file.  The mode defaults to overwrite the file.
        Create self._header to define the Header format
        in the HDF array.
        """
        self.filename = filename
        self.mode = mode
        self._header = np.zeros(1,
                                np.dtype([('BufferType', np.uint32),
                                          ('UTC', np.uint64),
                                          ('Index', np.uint64)]))

    def __enter__(self) -> object:
        """Context open of the file."""
        self._open()
        return self

    def __exit__(self, *args) -> None:
        """Context close of the HDF file."""
        self._close()

    @property
    def root(self):
        return self._hdf_file.root

    def create_table(self, name, dtype, title):
        return self._hdf_file.create_table(self.root,
                                           name,
                                           dtype,
                                           title=title,
                                           filters=self.filters)

    def _open(self) -> None:
        """
        Open the HDF file and define arrays.

        Open the file and create the two root
        arrays.  The Headers table holds pointers to
        the raw data taken by the instrument with
        time stamps and an integer type showing the data is
        from the ASIC DAQ.  Buffers hold the actual raw
        data from the instrument converted into <u32 integers.
        """
        self.filters = tables.Filters(complevel=1, complib='zlib')
        self._hdf_file = tables.open_file(
            self.filename, mode=self.mode, filters=self.filters)
        if 'w' in self.mode:
            self.buffers = self._hdf_file.create_vlarray(self.root,
                                                         'Buffers',
                                                         atom=tables.UInt32Atom(),
                                                         title='Ragged array of raw data buffer',
                                                         filters=self.filters)
            self.headers = self.create_table('Headers',
                                             self._header.dtype,
                                             title='Buffer header data')
        else:
            self.buffers = self.root.Buffers
            self.headers = self.root.Headers

    def _close(self) -> None:
        """Close the hdf file."""
        self._hdf_file.close()

    def write(self, utc, data, buffer_type=12) -> None:
        """
        Write a raw data buffer to the file.

        Initialize the header data and append it to file.
        Append the binary data to file and put it into
        a uint32 format.
        """
        self._header['BufferType'] = buffer_type
        self._header['UTC'] = utc
        self._header['Index'] = self.buffers.nrows
        self.headers.append(self._header)
        self.buffers.append(data.view('uint32'))

    def read(self):
        """Return all of the data at once."""
        return self.headers, self.buffers

    def __getitem__(self, index) -> Tuple[int, int, Iterable[int]]:
        """
        Allow iteration through the recorded buffers.

        Make the class iterable over the items already
        in the file.  Or allow for indexing to needed
        buffer.  Convert the data if necessary back into
        smaller elements.
        """
        header = self.headers[index]
        data = self.buffers[header['Index']]
        if header['BufferType'] == 12:  # ASIC data
            data = data.view('>u2')
        elif header['BufferType'] == 11:  # Bridgeport data
            data = data.view('u2')
        return header['BufferType'], header['UTC'], data



class AsicConvert:

	def __init__(self, file_glob,verbose=False):
		self.dtype = np.dtype([
			('asic', 'uint8'),
			('channel', 'uint16'),
			('adc', 'int16'),
			('tac', 'int16'),
			('oscillator', 'uint32'),
			('headers', 'uint16'),
			('dt','uint8')
		])
		#self.dtypedt = np.dtype([('dt','uint8')])
		self.verbose = verbose
		if self.verbose: print (file_glob)
		self.glob(file_glob)

	def glob(self, file_glob):
		if self.verbose: print(len(file_glob))
		if len(file_glob) == 0:
			f_glob = sorted(glob('data/*.hdf'))
		elif len(file_glob) == 1:
			path = pathlib.Path(file_glob[0])
			if path.is_dir():
				#print("path.is_dir")
				f_glob = sorted(glob(str(path / '*.hdf')))
			elif path.is_file():
				#print("path.is_file")
				f_glob = [str(path)]
			else:
				#print("else")
				f_glob = sorted([item for item in glob(str(path))])
		else:
			f_glob = sorted(
				[item for name in file_glob for item in glob(name)])

		print("Reading %d files..." % len(f_glob))
		parsed = []
		for filename in f_glob:
			print(filename)
			with Hdf(filename, 'r') as in_file:
				parse_data = np.concatenate([self.parse_buffer(data)
						for buffer_type, utc, data in in_file
						if buffer_type == 12])
				parsed.append(parse_data)
		self.numpy = np.concatenate(parsed)
		self.series = pandas.DataFrame(self.numpy)



	def parse_buffer(self, data):
		headers_orig = data[::4]
		osc_chan = data[1::4]
		index = data[2::4]
		adc_orig = data[3::4]
		#Need to use numpy shift operators we overflow the 16-bit artihmetic before casting to 32
		dt = (headers_orig >> 10) & 0x1
		oscillator = np.right_shift((osc_chan & 0xfff0), 4, dtype=('uint32')) + \
            np.left_shift((headers_orig & 0x00ff), 12, dtype=('uint32'))
		#oscillator = ((osc_chan & 0xfff0) >> 4) + \
		#    ((headers_orig & 0x00ff) << 12)
		headers = (headers_orig >> 8)
		asic = (osc_chan & 0x000f) >> 1
		channel = ((osc_chan & 0x0001) << 4) + ((index & 0xf000) >> 12)
		tac = ((index & 0x0fff) << 2) + ((adc_orig & 0xc000) >> 14)
		adc = (adc_orig & 0x3fff)
		np_array = np.core.records.fromarrays([
			asic, channel, adc, tac, oscillator, headers, dt],
			dtype=self.dtype)
		ind = np.where(osc_chan != 0xffff)
		return np_array[ind]

