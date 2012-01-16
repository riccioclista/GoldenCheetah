/*
 * Copyright (c) 2009 Mark Liversedge (liversedge@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
#include <QDebug>
#include "DeviceTypes.h"

// NOTE:
// Device Types not fully supported in this release (functionally) have
// been commented out. As new features or existing features are updated to
// use this new class then the lines will be uncommented
//
// As a result, only Realtime uses this feature at present (and the associated
// configuration DeviceConfiguration class and preferences pane

static DeviceType SupportedDevices[] =
{
#ifdef Q_OS_WIN32
      { DEV_ANTLOCAL, DEV_USB,     (char *) "Native ANT+",            true,    false,
        "ANT+ devices such as SRM, Powertap or Quarq power meters, Heart rate belts, "
        "speed or cadence meters via a Garmin ANT+ USB1 or USB2 stick",
        ":images/devices/garminusb.png" },
#else
      { DEV_ANTLOCAL, DEV_SERIAL,  (char *) "Native ANT+",           true,    false,
        "ANT+ devices such as SRM, Powertap or Quarq power meters, Heart rate belts, "
        "speed or cadence meters via a Garmin ANT+ USB1 or USB2 stick" ,
        ":images/devices/garminusb.png" },
#endif
      { DEV_CT,       DEV_SERIAL,  (char *) "Racermate Computrainer",true,    false,
        "Racermate Computrainer Lab or Pro bike trainer with the handlebar controller "
        "connected via a USB adaptor or directly connected to a local serial port." ,
        ":images/devices/computrainer.png"                                        },
#ifdef GC_HAVE_LIBUSB
      { DEV_FORTIUS,  DEV_LIBUSB,  (char *) "Tacx Fortius",          true,    false,
        "Tacx Fortius/iMagic bike trainer with the handlebar controller connected "
        "to a USB port. Please make sure you have device firmware to hand." ,
        ":images/devices/fortius.png" },
#endif
#if 0
      { DEV_GSERVER,  DEV_TCP,     (char *) "Golden Cheetah Server", false,   false,
        "Golden Cheetah racing server, not curently supported."                     },
      { DEV_NULL,     DEV_TCP,     (char *) "Null device (testing)", false,   false,
        "Testing device used for development only."                                 }.
#endif
#if 0 // deprecated, but keeping code until at least 3.1
      { DEV_ANTPLUS,  DEV_QUARQ,   (char *) "ANT+ via Quarqd",       true,    false,
        "ANT+ devices such as SRM, Powertap or Quarq power meters, Heart rate belts, "
        "speed or cadence meters via an existing Quarqd server" ,
        ":images/devices/quarqd.png" },
#endif

//    { DEV_PT,       DEV_SERIAL,  (char *) "Powertap Head Unit",    false,   true  },
//    { DEV_SRM,      DEV_SERIAL,  (char *) "SRM PowerControl V/VI", false,   true  },
//    { DEV_GCLIENT,  DEV_TCP,     (char *) "Golden Cheetah Client", false,   false },
      { 0, 0, NULL, 0, 0, "", "" }
};

DeviceTypes::DeviceTypes()
{
    for (int i=0; SupportedDevices[i].type;i++)
         Supported.append(SupportedDevices[i]);
}

DeviceTypes::~DeviceTypes()
{}

QList<DeviceType> DeviceTypes::getList()
{
    return Supported;
}

DeviceType DeviceTypes::getType(int type)
{
    for (int i=0; i< Supported.count(); i++) {
        if (Supported.at(i).type == type)
            return Supported.at(i);
    }
    return Supported.at(0); // yuck.whatever
}
