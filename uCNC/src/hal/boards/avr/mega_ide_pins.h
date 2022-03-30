/*
    Name: ide_mega_pins.h
    Description: Helper that converts IDE pins to µCNC pins.

    Copyright: Copyright (c) João Martins
    Author: João Martins
    Date: 29/03/2022

    µCNC is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version. Please see <http://www.gnu.org/licenses/>

    µCNC is distributed WITHOUT ANY WARRANTY;
    Also without the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the	GNU General Public License for more details.
*/

#ifndef MEGA_IDE_PINS_H
#define MEGA_IDE_PINS_H

#ifdef __cplusplus
extern "C"
{
#endif

#define IDE_PIND0_BIT 0
#define IDE_PIND1_BIT 1
#define IDE_PIND2_BIT 4
#define IDE_PIND3_BIT 5
#define IDE_PIND4_BIT 5
#define IDE_PIND5_BIT 3
#define IDE_PIND6_BIT 3
#define IDE_PIND7_BIT 4
#define IDE_PIND8_BIT 5
#define IDE_PIND9_BIT 6
#define IDE_PIND10_BIT 4
#define IDE_PIND11_BIT 5
#define IDE_PIND12_BIT 6
#define IDE_PIND13_BIT 7
#define IDE_PIND14_BIT 1
#define IDE_PIND15_BIT 0
#define IDE_PIND16_BIT 1
#define IDE_PIND17_BIT 0
#define IDE_PIND18_BIT 3
#define IDE_PIND19_BIT 9
#define IDE_PIND20_BIT 20
#define IDE_PIND21_BIT 21
#define IDE_PIND22_BIT 0
#define IDE_PIND23_BIT 1
#define IDE_PIND24_BIT 2
#define IDE_PIND25_BIT 3
#define IDE_PIND26_BIT 4
#define IDE_PIND27_BIT 5
#define IDE_PIND28_BIT 6
#define IDE_PIND29_BIT 7
#define IDE_PIND30_BIT 7
#define IDE_PIND31_BIT 6
#define IDE_PIND32_BIT 5
#define IDE_PIND33_BIT 4
#define IDE_PIND34_BIT 3
#define IDE_PIND35_BIT 2
#define IDE_PIND36_BIT 1
#define IDE_PIND37_BIT 0
#define IDE_PIND38_BIT 7
#define IDE_PIND39_BIT 2
#define IDE_PIND40_BIT 1
#define IDE_PIND41_BIT 0
#define IDE_PIND42_BIT 7
#define IDE_PIND43_BIT 6
#define IDE_PIND44_BIT 5
#define IDE_PIND45_BIT 5
#define IDE_PIND46_BIT 3
#define IDE_PIND47_BIT 2
#define IDE_PIND48_BIT 1
#define IDE_PIND49_BIT 0
#define IDE_PIND50_BIT 3
#define IDE_PIND51_BIT 2
#define IDE_PIND52_BIT 1
#define IDE_PIND53_BIT 0
#define IDE_PINA0_BIT 0
#define IDE_PINA1_BIT 1
#define IDE_PINA2_BIT 2
#define IDE_PINA3_BIT 3
#define IDE_PINA4_BIT 4
#define IDE_PINA5_BIT 5
#define IDE_PINA6_BIT 6
#define IDE_PINA7_BIT 7
#define IDE_PINA8_BIT 0
#define IDE_PINA9_BIT 1
#define IDE_PINA10_BIT 2
#define IDE_PINA11_BIT 3
#define IDE_PINA12_BIT 4
#define IDE_PINA13_BIT 5
#define IDE_PINA14_BIT 6
#define IDE_PINA15_BIT 7
#define PIN_BIT(X) IDE_PIN##X##_BIT

#define IDE_PIND0_PORT E
#define IDE_PIND1_PORT E
#define IDE_PIND2_PORT E
#define IDE_PIND3_PORT E
#define IDE_PIND4_PORT G
#define IDE_PIND5_PORT E
#define IDE_PIND6_PORT H
#define IDE_PIND7_PORT H
#define IDE_PIND8_PORT H
#define IDE_PIND9_PORT H
#define IDE_PIND10_PORT B
#define IDE_PIND11_PORT B
#define IDE_PIND12_PORT B
#define IDE_PIND13_PORT B
#define IDE_PIND14_PORT J
#define IDE_PIND15_PORT J
#define IDE_PIND16_PORT H
#define IDE_PIND17_PORT H
#define IDE_PIND18_PORT D
#define IDE_PIND19_PORT D
#define IDE_PIND20_PORT D
#define IDE_PIND21_PORT D
#define IDE_PIND22_PORT A
#define IDE_PIND23_PORT A
#define IDE_PIND24_PORT A
#define IDE_PIND25_PORT A
#define IDE_PIND26_PORT A
#define IDE_PIND27_PORT A
#define IDE_PIND28_PORT A
#define IDE_PIND29_PORT A
#define IDE_PIND30_PORT C
#define IDE_PIND31_PORT C
#define IDE_PIND32_PORT C
#define IDE_PIND33_PORT C
#define IDE_PIND34_PORT C
#define IDE_PIND35_PORT C
#define IDE_PIND36_PORT C
#define IDE_PIND37_PORT C
#define IDE_PIND38_PORT D
#define IDE_PIND39_PORT G
#define IDE_PIND40_PORT G
#define IDE_PIND41_PORT G
#define IDE_PIND42_PORT L
#define IDE_PIND43_PORT L
#define IDE_PIND44_PORT L
#define IDE_PIND45_PORT L
#define IDE_PIND46_PORT L
#define IDE_PIND47_PORT L
#define IDE_PIND48_PORT L
#define IDE_PIND49_PORT L
#define IDE_PIND50_PORT B
#define IDE_PIND51_PORT B
#define IDE_PIND52_PORT B
#define IDE_PIND53_PORT B
#define IDE_PINA0_PORT F
#define IDE_PINA1_PORT F
#define IDE_PINA2_PORT F
#define IDE_PINA3_PORT F
#define IDE_PINA4_PORT F
#define IDE_PINA5_PORT F
#define IDE_PINA6_PORT F
#define IDE_PINA7_PORT F
#define IDE_PINA8_PORT F
#define IDE_PINA9_PORT K
#define IDE_PINA10_PORT K
#define IDE_PINA11_PORT K
#define IDE_PINA12_PORT K
#define IDE_PINA13_PORT K
#define IDE_PINA14_PORT K
#define IDE_PINA15_PORT K
#define PIN_PORT(X) IDE_PIN##X##_PORT

#ifdef __cplusplus
}
#endif

#endif
