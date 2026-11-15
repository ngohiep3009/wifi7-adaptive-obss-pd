# Adaptive OBSS/PD Optimization for Wi-Fi 7 Networks

## 1. Overview

This project investigates adaptive OBSS/PD (Overlapping Basic Service Set Packet Detection) threshold adjustment for QoS-aware Wi-Fi 7 networks.

The objective is to improve network throughput while controlling latency and maintaining fair performance among neighboring BSSs under varying traffic conditions.

The project uses **ns-3.47** for network simulation.

## 2. Research Objectives

- Investigate OBSS/PD and Spatial Reuse mechanisms.
- Develop a Wi-Fi 7 simulation environment with overlapping BSSs.
- Evaluate network performance under different fixed OBSS/PD thresholds.
- Design an adaptive OBSS/PD threshold adjustment mechanism.
- Compare the proposed mechanism with baseline approaches.

## 3. Current Simulation Model

The initial simulation includes:

- Wi-Fi standard: IEEE 802.11be (Wi-Fi 7).
- Two overlapping BSSs.
- Two access points and six stations.
- Shared wireless channel: 5 GHz, channel 36, 20 MHz.
- BSS Coloring.
- UDP downlink traffic.
- Constant OBSS/PD algorithm.
- FlowMonitor for QoS measurements.

### Performance Metrics

- Throughput
- Delay
- Jitter
- Packet delivery and loss statistics
- OBSS/PD reset events

## 4. Project Structure

```text
wifi7-adaptive-obss-pd/
├── simulations/
│   └── wifi7-two-bss.cc
├── analysis/
├── results/
├── docs/
├── README.md
└── .gitignore
```

## 5. Running the Simulation

Requirements:

- Ubuntu/Linux
- ns-3.47
- C++ compiler

Copy the simulation file into the ns-3 `scratch/` directory:

```bash
cp simulations/wifi7-two-bss.cc /path/to/ns-3-dev/scratch/
```

Run the simulation from the ns-3 root directory:

```bash
./ns3 run scratch/wifi7-two-bss
```

## 6. Current Progress

**Completed:**
- Two-BSS Wi-Fi 7 topology.
- BSS Coloring and shared-channel configuration.
- UDP traffic generation.
- Throughput measurements.
- FlowMonitor integration.
- OBSS/PD reset-event monitoring.

**In Progress:**
- Verification of OBSS/PD activation.
- Fixed-threshold performance evaluation.
- Traffic variation scenarios.

**Planned:**
- Adaptive threshold adjustment algorithm.
- QoS optimization and comparative evaluation.

## 7. Research Status

This repository currently contains an initial simulation baseline.

The adaptive OBSS/PD algorithm has not yet been implemented, and the effectiveness of Spatial Reuse has not yet been experimentally verified.
