## Plasma Vault

Plasma applet and services for creating encrypted vaults.

### Getting Started

 - open the Plasma Vault applet from the System Tray (it may be hidden in the "Show hidden icons" popup)
 - click "Create a New Vault"
 - follow the vault creation wizard

Plasma Vault ships with a Dolphin plugin which allows locking and unlocking vaults from within Dolphin: right-click the vault's mountpoint (in `~/Vaults` by default) in Dolphin, and select the corresponding context menu option.

### CLI Usage

Mounting / unlocking a vault:

```sh
qdbus org.kde.kded6 /modules/plasmavault org.kde.plasmavault.openVault "<absolute path of encrypted backing folder>"
```

Unmounting / re-locking a vault:

```sh
qdbus org.kde.kded6 /modules/plasmavault org.kde.plasmavault.closeVault "<absolute path of encrypted backing folder>"
```

Note: depending on your distro, `qdbus` may be named `qdbus6` or `qdbus-qt6`.

### Migrating old mountpoints

Plasma Vault will automatically "lock" mountpoints when their corresponding vault is unmounted, preventing files from being saved their accidentally.

By default, only vaults created with Plasma 6.7 or later are opted into this feature; to enable it for an existing mountpoint, create a file named `.directory` within the vault's mountpoint with the following contents:

```
[Desktop Entry]
Icon=folder-encrypted
X-KDE-LockVaultMountPoint=true
```

Usage of this feature currently requires the gocryptfs backend.