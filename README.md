# victus-power-ctrl
An issue with my victus laptop I've been having is that with performance mode enabled on my HP Victus 
16-s0177ng and with fans at max speed (shout-out to betelqeyza and Vilez0), CPU jumps immediately up to 
95-98 C. This service constantly reads current CPU temp and adjusts power profile to appropriate thermal
zone.

As there are no fully functioning fan-curve service (Vilez0's implementation only increases fan speed, but doesn't decreese), I implemented simple fan curve for both cpu and gpu. 

Note: if you have other governor's installed, they will still work. RyzenAdj is a really nice utility that I highly recommend. 

# Requirements
- `systemd` as service manager
- `powerprofilectl` controlling power profiles
- `hp-wmi` by Vilez0 driver
- nvidia drivers installed (check with `nvidia-smi`)

# Installation 
```bash
git clone https://github.com/OneGraund/victus-power-ctrl.git
cd victus-power-ctrl
sudo make service_install
sudo make service_start
```

# Uninstall
```bash
sudo make service_stop
sudo make service_rm
```

### Logs
View journalctl messages regarding service via:
```bash
make service_logs
```
Log level can be changed in `utils.h`

# Development
To run in terminal do:
```bash
sudo make run
```

# To-Do
- [ ] extend curves settings 
  - [ ] add configuration of fan curves using config file
  - [ ] add new graphs resembling curves (maybe ask user to use some website?)
- [ ] add cli tool
  - [ ] enable/disable panic-mode
  - [ ] overwrite fan mode "on-the-go"
- [ ] keyboard color ctrl too??

# Issues
