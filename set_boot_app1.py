Import("env")
import subprocess
import sys

def after_upload(source, target, env):
    """Set OTA data to boot app1 after upload"""
    print("Setting OTA data to boot from app1...")

    upload_port = env.get("UPLOAD_PORT", "COM6")

    # Run esptool to set OTA data
    result = subprocess.run([
        sys.executable, "-m", "esptool",
        "--port", upload_port,
        "write_flash", "0xE000", "otadata_boot_app1.bin"
    ], capture_output=True, text=True)

    if result.returncode == 0:
        print("✓ OTA data set to boot app1")
        print("✓ Device will boot custom firmware on next reset")
    else:
        print(f"✗ Failed to set OTA data: {result.stderr}")
        sys.exit(1)

# Register the callback
env.AddPostAction("upload", after_upload)
