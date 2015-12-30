/*
 * VexIME.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: bouchier
 */
/*
 The following is a description of LED patterns:
 ==============================================

 Yellow - not initialized (default Address)

 Every 3 sec
   Slow Green Blip - initialized and encoder is not changing
   Slow Green Double Blip - initialized, neutral and terminated
   Slow Green Micro Blip - initialized, device is idle

 Blinking Green - relative to speed
 Solid Green - full speed in either direction

 Yellow (same as Green patterns) - counter overflow

 Solid Red - data is being written to EEprom (1sec)
 Blinking Red - address range error (a valid address will clear error)

 Velocity Bits:
 =============
 The "velocity bits" are really delta-time per encoder gear revolution, so 1/speed.
 The units are in 64 microsecond tics per encoder revolution for the 269 IME and 64
 microsecond tics per encoder half-revolution for the 393 IME. The values increase as
 the rotation rate slows, with stopped for more than 4 seconds being reported as 0xFFFF.
 The values are only Magnitude and do not include a Sign bit for direction.
 Doing sequential Rotation reads can get you direction.

 For the 269 IME, the encoder wheel turns 30.056 times per output revolution.
 To get velocity bits into rpm, do the following. Multiple the value (in tics / encoder
 rev) times 0.000064 (seconds / tic) to get a value in (seconds / encoder rev). Take the
 reciprocal to get a value in (encoder rev / second). Multiply by 60 (seconds / minute)
 and 1/30.056 (output rev / encoder rev) to get your value in (output rev / minute).

 For the 393 IME with the factory (torque) gearing, the encoder wheel turns 39.2 times per
 output revolution. To get velocity bits into rpm, do the following. Multiple the value
 (in tics / half-encoder rev) times 0.000064 (seconds / tic) and times 2 (half-encoder
 rev / encoder-rev) to get a value in (seconds / encoder rev). Take the reciprocal to get
 a value in (encoder rev / second). Multiply by 60 (seconds / minute) and 1/39.2 (output
 rev / encoder rev) to get your value in (output rev / minute).

 For the 393 IME with the speed gearing, the encoder wheel turns 24.5 times per output
 revolution. To get velocity bits into rpm, do the following. Multiple the value (in tics
 / half-encoder rev) times 0.000064 (seconds / tic) and times 2 (half-encoder rev / encoder-rev)
 to get a value in (seconds / encoder rev). Take the reciprocal to get a value in (encoder rev
 / second). Multiply by 60 (seconds / minute) and 1/24.5 (output rev / encoder rev) to get
 your value in (output rev / minute).
 */

#include "VexIME.h"

VexIME::VexIME() {
	// TODO Auto-generated constructor stub

}

VexIME::~VexIME() {
	// TODO Auto-generated destructor stub
}

#if 0
VexIME::VexIME(CQEI2C i2c) {
}
}
#endif
