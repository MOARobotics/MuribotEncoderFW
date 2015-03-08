# Muribot Encoder Firmware
**A part of the Muribot Project**

## Description

This is the production version of the encoder firmware contained on the Muribot Educational Robotics Platform. It is based around a [PIC12F1822/PIC12LF1822], but the general principals of its operation can be applied to any microcontroller.

## How It Works
####I2C
I2C is a simple two wire protocol seen in many embedded projects. This protocol allows multiple devices to share the same bus, and unsurprisingly Muribot does as well. The specifics of the protocol are beyond this article, as they are better explained by the [Wikipedia Article][1]. The firmware responds to read and write commands the following way:

#####Read Command
Upon receiving a read command the software will immediately stream out the encoder data, starting with the most significant bytes, in the following format:

*[ 4B Left Counts ][ 4B Right Counts ][ 1B Left Direction ][ 1B Right Direction]*

The counts are stored as **32-bit signed integers**, this data-type allows a range of **+/-2,147,483,647** counts. A wheel will generate 180 counts per revolution of the wheel at the time of this writing.The wheels on the Muribot have a diameter of ~32mm, multiplying that by Pi gives us the circumference: **100.5309mm**. By dividing the **data-type range** by **180**, and multiplying the result by **100.5309** we can see how far the Muribot would have to go to cause an **overflow** of the counts, which is strictly forbidden.

>**Comparison of 8-bit C data types**<br>
>*Byte - 1B*<br>Range: +/-127<br>Max Distance: 70.9301mm<br><br>
>*Int - 2B*<br>Range: +/-32,767<br>Max Distance: 18.3005M<br><br>
>*Long Int - 4B*<br>Range: +/-2,147,483,647<br>Max Distance: 1199.3803KM

#####Write Command
Upon receiving a write command, the software will reset the encoder module and all counts and directions are zeroed out.

####Interrupt-on-Change
The Muribot uses the 8-pin [PIC12F1822/PIC12LF1822], 2 pins are used for power while another 2 are used for the I2C bus. The remaining 4 pins (RA0, RA3-RA5) are directly connected to the sensors used to read the encoder discs. As the encoder disc passes in front of the two sensors, they will generate two signals that are ~90° out of phase with each other, this can be used to tell which direction the wheel is traveling; a method known as [quadrature encoding]. A demo written in Processing is available if you want to know more about how the Muribot utilizes it.

[Processing Demo]

We make use of a hardware feature present on most microcontrollers where an interrupt is generated when a pin changes state. We watch for these events, and when one happens we increment or decrement the counts and update the directions accordingly.

## Version
v1.0

## Todo
None!

## License
[GPL v3.0]

[Mid-Ohio Area Robotics]**, Electronics for Everyone!**

[PIC12F1822/PIC12LF1822]:http://www.microchip.com/wwwproducts/Devices.aspx?dDocName=en544839
[Mid-Ohio Area Robotics]:http://www.moarobotics.com/
[GPL v3.0]:http://www.gnu.org/licenses/gpl-3.0.txt
[quadrature encoding]:http://en.wikipedia.org/wiki/Rotary_encoder#Incremental_rotary_encoder
[Processing Demo]:http://www.openprocessing.org/sketch/188462
[1]:http://en.wikipedia.org/wiki/I%C2%B2C