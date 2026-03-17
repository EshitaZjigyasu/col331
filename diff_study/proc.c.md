# proc.c Changes

## Overview
The helper `mycpu` was simplified for a uniprocessor environment.

## Detailed Changes
- `mycpu` now simply returns `&cpus[0]` instead of performing APIC ID lookup and checking interrupt flags.

## Impact
This reflects the assumption of a single CPU, simplifying the code and removing unnecessary complexity. It has no effect on functionality in the current build but would need revision for multiprocessor support.