J. Beechert

ecal_Wanaka2020.ecal:
Energy calibration of COSI data taken in Wanaka 2020. File created on ~August 20, 2020.
Am-241, Ba-133, Co-57 data taken on the bench for all detectors.
Co-60, Na-22, Y-88, and Cs-137 data taken with the cryostat integrated for all detectors.


End of Sept. 2020, I realized that some of the high FWHMs on detectors 3, 8, and 9 could
be improved by using Am-241, Ba-133, and Co-57 data taken with the cryostat integrated instead
of data taken on the bench, as we usually do with those three sources. So,

ecal_Wanaka2020_201005.ecal:
File created on October 5, 2020.
New energy calibration identical to ecal_Wanaka2020.ecal EXCEPT for the calibrations of
detectors 3, 8, and 9, which in this file used only data taken with the cryostat integrated. All
other detectors used bench data for Am-241, Ba-133, and Co-57 and cryostat-integrated data for the 
other sources.

Because this file may better reflect the FWHMs COSI sees in its nominal flight configuration (ie, integrated),
at least on detectors 3, 8, 9, use this file for COSI analysis instead of ecal_Wanaka2020.ecal. 

However, if you need the 81 keV peak on all detectors, you will probably have to use ecal_Wanaka2020.ecal instead
because the cryostat-integrated data on detectors 3, 8, and 9 don't have 81 keV results (poor exposure).

The new calibrations on detectors 3, 8, and 9 in this new .ecal file barely impact the energy resolution numbers
seen in the original ecal_Wanaka2020.ecal. I really think the only visible changes are in the FWHMs on 
the troublesome strips of detectors 3, 8, and 9, which comprise a limited fraction of COSI's 888 strips.
