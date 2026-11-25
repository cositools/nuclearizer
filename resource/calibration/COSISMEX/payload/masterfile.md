# Masterfile

This masterfile describes which calibration files should be used together in the nuclearizer/DEE pipeline:

`v1 (latest)`:
- `stripmap/strip-map-FM-config2b-20261002.map`
- `stripmap/disabled-strips-FM-config2b-20261002.csv`
- `ecal/full-ecal-poly2-FM-config2b-20261020.ecal` <!-- 20261020_detall_noAm241_poly2.ecal -->
- `taccal/taccal-FM-config2b-20261005.csv`  <!-- taccal_20261005_1747d.csv -->
- `simulations/depth-splines-FM-config2b-20261125.csv` <!-- has Q0L2 at 500V and Q3L1 at 600V, which is what the depth cal expects --> 
- `depth/depth-coeffs-FM-config2b-20261026.csv` <!-- 20261026_depth_Cs137_partyhat_ecalV1_taccalV0_threshNom_detAll.csv -->
- `thresholds/hardware-thresholds-FM-config2b-20261120.csv`


`derived-from-unitlevel`:
- `stripmap/strip-map-derived-from-unitlevel.map`
- `stripmap/disabled-strips-derived-from-unitlevel.csv`
- `ecal/full-ecal-derived-from-unitlevel.ecal`
- `taccal/taccal-derived-from-unitlevel.csv`
- `simulations/depth-splines-derived-from-unitlevel.csv`
- `depth/depth-coeffs-derived-from-unitlevel.csv`
- `thresholds/hardware-thresholds-derived-from-unitlevel.csv`