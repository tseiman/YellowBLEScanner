# Yellow BLE Scanner
This Legato Component makes use of the Bluetooth/WIFI Chip on the MangOH Yellow Dev Kit (https://mangoh.io/mangoh-yellow). 
The idea is to scan BLE Advertisments and report them into Sierra Wireless Octave system.

## How To Build
1. You need a running MangoOH Yellow System build environment with Octave enabled follow the advises from here https://forum.mangoh.io/t/leaf-packages-for-yellow/2979/5

2. clone this project 
```bash
$ git clone https://github.com/tseiman/YellowBLEScanner.git
```

3. enter the project folder 
```bash
$ cd YellowBLEScanner
```

4. search for the latest mangOH yellow with octave version 
```bashe 
$ leaf search -t mangOH
``` 
this should be the same version which was used in step 1. for the mangOH yellow system build

5. do for the **YellowBLEScanner** project a leaf setup - e.g.: 
```bashe
$ leaf setup -p mangOH-yellow-wp77xx_0.2.3
``` 
(substitute the version accordingly)

6. start the leaf shell 
```bash
$ leaf shell
```

7. set an enviroment variable to the MangOH Build environment from step 1. - e.g.: 
```bash
$ export MY_MANGOH_YELLOW_SYSTEM=/home/user/mangOH-workspace/mangOH/
```

8. build the application: 
```bash
mkapp ${LEGATO_DEF_FILE} -s components -t ${LEGATO_TARGET} -w ${LEGATO_OBJECT_DIR} \
-o ${LEGATO_OUTPUT_DIR} --interface-search=${LEGATO_ROOT}/apps/sample/dataHub \
--component-search=${MY_MANGOH_YELLOW_SYSTEM}/components/  YellowBLEScanner.adef
```

9. install the app 
```bash
$ update YellowBLEScanner.wp77xx.update 192.168.2.2
```
