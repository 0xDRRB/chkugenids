# chkugenids
A little tool to get informations about USB devices from /dev/ugenN.EE on NetBSD.

The main goal of this program is to display informations about USB generic device so you can use them with `devpubd` to setup symlinks, permissions, and so on.

The *ugen* driver provides support for all USB devices that do not have a special driver, like USB NFC Readers, and when no other driver attaches to a device. `chkugenids` can be used in `devpubd` scripts to check for VID:PID, vendor name, product name, serial number... and act accordingly.

Exemple: Here we have an ASK LoGO NFC reader (usable with LibNFC) :

```
$ sudo chkugenids -f /dev/ugen1.00 -a
IDs     : 1fd3:0608
Vendor  : ASK
Product : LoGO
Serial  :
Bus     : 3
Address : 2
Class   : 0
Sudclass: 0
Power   : 300 mA
Speed   : 2 (Full Speed - 12 Mbps)
```

We can quickly get VID and PID :

```
$ sudo chkugenids -f /dev/ugen1.00 -i
1fd3:0608
```

or all informations on one line (to use with `cut` and `grep`) :

```
$ sudo ./chkugenids -f /dev/ugen1.00 -l
1fd3:0608:ASK:LoGO::3:2:0:0:300:2
```

Let's say we want users from group `wheel` to be able to read and write this device without `sudo`. Just add `devpubd=YES` in your `/etc/rc.conf` and create a script in `/libexec/devpubd-hooks`. Something like a `04-NFCsetperm` containing :

```
#!/bin/sh
#
# Change permissions on /dev/ugenN.EE
#

event="$1"
shift
devices=$@

for device in $devices; do
   case $device in
   ugen*)
      case $event in
      device-attach)
         echo "NFCsetperm: $device attached"
         chkugenids -f /dev/$device.00 -i -q | grep "1fd3:0608" > /dev/null
         if [ $? -eq 0 ]; then
            logger -s "NFCsetperm: device match. Adjusting permissions for /dev/$device.*"
            chmod g+rw /dev/$device.*
         fi
         ;;
      device-detach)
         logger -s "NFCsetperm: $device detached. Setting back permissions on /dev/$device.*!"
         chmod 600 /dev/$device.*
         ;;
      esac
      ;;
   esac
done
```

When the reader device is plugged, `/dev/ugenN.EE` is created by `/libexec/devpubd-hooks/01-makedev`, then `devpubd` call `04-NFCsetperm` where `chkugenids` is used to check for VID:PID and set `g+rw` to `/dev/ugenN.*` if we have a match.

