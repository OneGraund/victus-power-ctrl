# victus-power-ctrl
An issue with my victus laptop I've been having is that with performance mode enabled on my HP Victus 
16-s0177ng and with fans at max speed (shout-out to betelqeyza and Vilez0), CPU jumps immediately up to 
95-98 C. This service constantly reads current CPU temp and adjusts power profile to appropriate thermal
zone.

# Requirements
- `systemd` as service manager
- `powerprofilectl` controlling power profiles

# Installation 
```bash
git clone https://github.com/OneGraund/victus-power-ctrl.git
cd victus-power-ctrl
make service_install
```

# Development
To run in terminal do:
```bash
make run
```


# To-Do
- dynamically change threshholds not to fry cpu
- add `.sh` sciprt to install as service
    - create make command to run it