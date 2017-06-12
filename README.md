# tzme
A GeoIP-based tool for automatically updating the system timezone

### Dependencies

* [libcurl](https://curl.haxx.se/libcurl/): The multiprotocol file transfer
    library
* [libsystemd](https://www.freedesktop.org/wiki/Software/systemd/): System and
    Service Manager
* [libxml2](http://xmlsoft.org/): The XML C parser and toolkit of Gnome

### Building

Building is simple!

```bash
cd tzme/
make
```

### Usage

_Note_, you probably will need to run this as `root` to make the permissions
work out.

```
usage: tzme [timezone]

Dynamically update the system timezone based on GeoIP

Args:
  timezone: Optional, pass a timezone to set manually
    See: https://www.freedesktop.org/wiki/Software/systemd/timedated/
```

## Arch Linux

`tzme` is available as an [Arch Linux](https://www.archlinux.org/) package
via the [AUR](https://aur.archlinux.org/).

See the package [here](https://aur.archlinux.org/packages/tzme-git/)!

### Automatic updates on internet connection

With `systemd` integration, comes the ability to have the timezone update
whenever an internet connection comes up. Simply enable the installed service:

```bash
systemctl enable tzme.service
```

Note, you may need some kind of "shim" to re-trigger the `network-online`
target when the system has a verifiable internet connection, otherwise `tzme`
fails silently!
