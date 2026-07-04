# Release Procedure

Masareef releases are automated via GitHub Actions, triggered by pushing a version tag.

## One-time Setup

### 1. Generate an Android keystore

```bash
keytool -genkeypair \
  -keystore masareef-release.keystore \
  -alias masareef-key \
  -keyalg RSA \
  -keysize 4096 \
  -validity 10950 \
  -dname "CN=Your Name, O=Your Org, L=City, ST=State, C=Country"
```

When prompted, enter and confirm a strong password for both the keystore and the key (can be the same).

### 2. Encode keystore to base64

```bash
base64 -w 0 masareef-release.keystore > keystore.b64
cat keystore.b64  # copy the output
```

### 3. Configure GitHub repository secrets

Set these secrets in your GitHub repository settings (Settings → Secrets and variables → Actions):

- **ANDROID_KEYSTORE_B64**: Paste the base64-encoded keystore from step 2
- **ANDROID_KEYSTORE_PASS**: The keystore password from step 1
- **ANDROID_KEY_ALIAS**: The key alias from step 1 (e.g., `masareef-key`)
- **ANDROID_KEY_PASS**: The key password from step 1

## Releasing a New Version

### 1. Update the version in CMakeLists.txt

Edit `CMakeLists.txt` and update the `project()` line:

```cmake
project(masareef VERSION X.Y.Z LANGUAGES CXX)
```

Commit this change:

```bash
git add CMakeLists.txt
git commit -m "Bump version to X.Y.Z"
```

### 2. Create and push a release tag

```bash
git tag -a vX.Y.Z -m "Release version X.Y.Z"
git push origin vX.Y.Z
```

The tag **must** match the version in `CMakeLists.txt` (prefixed with `v`). For example:
- CMakeLists.txt: `1.2.3`
- Tag: `v1.2.3`

### 3. Monitor the release workflow

The release workflow will:
1. **Guard**: Verify the tag matches the CMakeLists.txt version
2. **AppImage**: Build the desktop Linux app and package as `.AppImage`
3. **APK**: Build and sign the Android app as `.apk`
4. **Release**: Download both artifacts and publish them with GitHub Releases

Once the workflow completes, the release will be visible at:
```
https://github.com/your-org/masareef/releases/tag/vX.Y.Z
```

## Release Artifacts

Each release includes:
- **Masareef-X.Y.Z-x86_64.AppImage**: Desktop Linux application (portable executable)
- **masareef-release-arm64-v8a.apk**: Android app for ARM 64-bit devices (virtually all modern phones)
- **sha256sums.txt**: SHA256 checksums for verifying artifact integrity

## Android Version Code

The Android version code is computed automatically from the semantic version:

```
version_code = major*10000 + minor*100 + patch
```

For example:
- `1.0.0` → `10000`
- `1.2.3` → `10203`
- `2.5.10` → `20510`

This ensures monotonically increasing version codes, which Android requires for APK upgrades.
