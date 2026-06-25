/**
 * ProcEmulator - Realistic /proc/ filesystem emulation
 */

#include "anti_detect/ProcEmulator.hpp"
#include "core/CryptoUtils.hpp"
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

namespace VirtualPhonePro {

ProcEmulator::ProcEmulator() 
    : m_bootTime(0) {
}

ProcEmulator::~ProcEmulator() {}

ProcEmulator& ProcEmulator::getInstance() {
    static ProcEmulator instance;
    return instance;
}

void ProcEmulator::initialize(const std::string& manufacturer, const std::string& model) {
    m_manufacturer = manufacturer;
    m_model = model;
    
    if (manufacturer == "Samsung") {
        loadSamsungCPUConfig();
    } else if (manufacturer == "Google") {
        loadGoogleCPUConfig();
    } else if (manufacturer == "Xiaomi") {
        loadXiaomiCPUConfig();
    } else if (manufacturer == "OnePlus") {
        loadOnePlusCPUConfig();
    } else {
        loadSamsungCPUConfig(); // Default
    }
    
    // Set boot time to random past time (realistic)
    m_bootTime = time(nullptr) - (Crypto::SecureRandomGenerator().generateUint32() % 864000); // 0-10 days ago
}

void ProcEmulator::loadSamsungCPUConfig() {
    m_cpuHardware = "qcom";
    m_cpuConfig.processor = "proc-，红";
    m_cpuConfig.hardware = "Hardware: Qualcomm Technologies, Inc SM8250-or-SM8150";
    m_cpuConfig.implementer = "0x51"; // Qualcomm
    m_kernelConfig.version = "5.10.101-android12-9-";
    m_kernelConfig.linuxVersion = "5.10.101-android12-9-";
    m_kernelConfig.compiler = "aarch64-linux-android-gcc (GCC) 9.0 20191127";
    m_kernelConfig.buildDate = "Thu Mar 16 20:33:57 UTC 2023";
}

void ProcEmulator::loadGoogleCPUConfig() {
    m_cpuHardware = "qcom";
    m_cpuConfig.processor = "proc";
    m_cpuConfig.hardware = "Hardware: Google Tensor";  
    m_cpuConfig.implementer = "0x48"; // Google
    m_kernelConfig.version = "5.10.43-android13-7-";
    m_kernelConfig.linuxVersion = "5.10.43-android13-7-";
    m_kernelConfig.compiler = "aarch64-linux-android-gcc (GCC) 11.1 20220413";
    m_kernelConfig.buildDate = "Wed Apr 19 14:22:11 UTC 2023";
}

void ProcEmulator::loadXiaomiCPUConfig() {
    m_cpuHardware = "qcom";
    m_cpuConfig.processor = "proc";
    m_cpuConfig.hardware = "Hardware: Qualcomm Technologies, Inc lahaina"; // Snapdragon 888
    m_cpuConfig.implementer = "0x51";
    m_kernelConfig.version = "5.10.101-android13-5-";
    m_kernelConfig.linuxVersion = "5.10.101-android13-5-";
    m_kernelConfig.compiler = "aarch64-linux-android-gcc (GCC) 9.0 20191127";
    m_kernelConfig.buildDate = "Fri Jan 20 08:15:30 UTC 2023";
}

void ProcEmulator::loadOnePlusCPUConfig() {
    m_cpuHardware = "qcom";
    m_cpuConfig.processor = "proc";
    m_cpuConfig.hardware = "Hardware: Qualcomm Technologies, Inc joyeuse"; // Snapdragon 870
    m_cpuConfig.implementer = "0x51";
    m_kernelConfig.version = "5.10.77-android12-9-";
    m_kernelConfig.linuxVersion = "5.10.77-android12-9-";
    m_kernelConfig.compiler = "aarch64-linux-android-gcc (GCC) 9.0 20191127";
    m_kernelConfig.buildDate = "Mon Feb 27 11:45:22 UTC 2023";
}

void ProcEmulator::setDeviceInfo(const std::string& manufacturer, const std::string& model, const std::string& hardware) {
    m_manufacturer = manufacturer;
    m_model = model;
    m_hardware = hardware;
    initialize(manufacturer, model);
}

void ProcEmulator::setBootTime(time_t bootTime) {
    m_bootTime = bootTime;
}

std::string ProcEmulator::getCpuInfo() {
    std::stringstream ss;
    Crypto::SecureRandomGenerator rng;
    
    // Processor 0
    ss << "processor\t: 0\n";
    ss << "BogoMIPS\t: " << (48 + rng.generateUint32() % 4) << "\n";
    ss << "Features\t: " << generateFeatures() << "\n";
    ss << "CPU implementer\t: " << m_cpuConfig.implementer << "\n";
    ss << "CPU architecture: 8\n";
    ss << "CPU variant\t: 0x" << std::hex << std::setw(2) << std::setfill('0') 
       << (0x10 + rng.generateUint32() % 0xF0) << std::dec << "\n";
    ss << "CPU part\t: 0x" << std::hex << std::setw(3) << std::setfill('0') 
       << (0x0D05 + rng.generateUint32() % 0x100) << std::dec << "\n";
    ss << "CPU revision\t: " << (rng.generateUint32() % 10) << "\n";
    ss << "Processor\t: " << m_cpuConfig.processor << "\n";
    ss << "Hardware\t: " << m_cpuHardware << "\n";
    
    // Add more cores
    for (int i = 1; i < 8; i++) {
        ss << "\nprocessor\t: " << i << "\n";
        ss << "BogoMIPS\t: " << (48 + rng.generateUint32() % 4) << "\n";
        ss << "Features\t: " << generateFeatures() << "\n";
        ss << "CPU implementer\t: " << m_cpuConfig.implementer << "\n";
        ss << "CPU architecture: 8\n";
        ss << "CPU variant\t: 0x" << std::hex << std::setw(2) << std::setfill('0') 
           << (0x10 + rng.generateUint32() % 0xF0) << std::dec << "\n";
        ss << "CPU part\t: 0x" << std::hex << std::setw(3) << std::setfill('0') 
           << (0x0D05 + rng.generateUint32() % 0x100) << std::dec << "\n";
        ss << "CPU revision\t: " << (rng.generateUint32() % 10) << "\n";
        ss << "Processor\t: " << m_cpuConfig.processor << "\n";
        ss << "Hardware\t: " << m_cpuHardware << "\n";
    }
    
    // Revision
    ss << "Revision\t: " << std::hex << std::setw(4) << std::setfill('0') 
       << (0x1000 + rng.generateUint32() % 0xE000) << std::dec << "\n";
    ss << "Serial\t\t: " << generateRandomHex(8) << "\n";
    
    return ss.str();
}

std::string ProcEmulator::generateFeatures() {
    return "fp asimd evtstrm aes pmull sha1 sha2 crc32 cpuid";
}

std::string ProcEmulator::getVersion() {
    std::stringstream ss;
    ss << m_kernelConfig.version << " #1 SMP PREEMPT " << m_kernelConfig.buildDate << "\n";
    ss << "(" << m_kernelConfig.compiler << ") #1 GCC " << m_kernelConfig.compiler << "\n";
    return ss.str();
}

std::string ProcEmulator::getUptime() {
    if (m_bootTime == 0) {
        m_bootTime = time(nullptr) - (3600 + Crypto::SecureRandomGenerator().generateUint32() % 86400);
    }
    
    time_t now = time(nullptr);
    double uptime = difftime(now, m_bootTime);
    
    // Idle time (some cores sleeping)
    double idle = uptime * 0.7;
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << uptime << " " << idle;
    return ss.str();
}

std::string ProcEmulator::getMemInfo() {
    std::stringstream ss;
    ss << "MemTotal:        7800000 kB\n";
    ss << "MemFree:         2100000 kB\n";
    ss << "MemAvailable:    5400000 kB\n";
    ss << "Buffers:           180000 kB\n";
    ss << "Cached:           2400000 kB\n";
    ss << "SwapCached:             0 kB\n";
    ss << "Active:           2800000 kB\n";
    ss << "Inactive:         1600000 kB\n";
    ss << "Active(anon):     1400000 kB\n";
    ss << "Inactive(anon):    400000 kB\n";
    ss << "Active(file):     1400000 kB\n";
    ss << "Inactive(file):   1200000 kB\n";
    ss << "Unevictable:        32000 kB\n";
    ss << "Mlocked:            32000 kB\n";
    ss << "SwapTotal:        2000000 kB\n";
    ss << "SwapFree:         2000000 kB\n";
    ss << "Dirty:                  0 kB\n";
    ss << "Writeback:               0 kB\n";
    ss << "AnonPages:        1400000 kB\n";
    ss << "Mapped:            600000 kB\n";
    ss << "Shmem:             120000 kB\n";
    ss << "KReclaimable:      300000 kB\n";
    ss << "Slab:              380000 kB\n";
    ss << "SReclaimable:      300000 kB\n";
    ss << "SUnreclaim:         80000 kB\n";
    ss << "KernelStack:        32000 kB\n";
    ss << "PageTables:         40000 kB\n";
    ss << "NFS_Unstable:            0 kB\n";
    ss << "Bounce:                  0 kB\n";
    ss << "WritebackTmp:            0 kB\n";
    ss << "CommitLimit:      5900000 kB\n";
    ss << "Committed_AS:     4200000 kB\n";
    ss << "VmallocTotal:   263168000 kB\n";
    ss << "VmallocUsed:       100000 kB\n";
    ss << "VmallocChunk:          0 kB\n";
    ss << "Percpu:             2000 kB\n";
    ss << "CmaTotal:         1048576 kB\n";
    ss << "CmaFree:           800000 kB\n";
    return ss.str();
}

std::string ProcEmulator::getMounts() {
    std::stringstream ss;
    ss << "rootfs / rootfs ro,seclabel,size=1933524k,nr_inodes=238609 0 0\n";
    ss << "tmpfs /dev tmpfs rw,seclabel,nosuid,relatime,size=1933524k,nr_inodes=238609,mode=755 0 0\n";
    ss << "devpts /dev/pts devpts rw,seclabel,relatime,mode=600 0 0\n";
    ss << "proc /proc proc rw,relatime,gid=3009,hidepid=2 0 0\n";
    ss << "sysfs /sys sysfs rw,seclabel,relatime 0 0\n";
    ss << "selinuxfs /sys/fs/selinux selinuxfs rw,relatime 0 0\n";
    ss << "/dev/block/dm-0 /system ext4 ro,seclabel,relatime 0 0\n";
    ss << "/dev/block/dm-1 /product ext4 ro,seclabel,relatime 0 0\n";
    ss << "/dev/block/dm-2 /vendor ext4 ro,seclabel,relatime 0 0\n";
    ss << "/dev/block/sda45 /data f2fs rw,seclabel,relatime,background_gc=on,discard_handler,sysfsrc,native_io,extension=pdf,user_xattr 0 0\n";
    ss << "/dev/block/sda46 /metadata ext4 rw,seclabel,nosuid,noatime 0 0\n";
    ss << "tmpfs /mnt/obb tmpfs rw,seclabel,nosuid,nodev,size=1933524k,nr_inodes=238609 0 0\n";
    ss << "tmpfs /apex tmpfs rw,seclabel,nosuid,nodev,noatime 0 0\n";
    return ss.str();
}

std::string ProcEmulator::getStatus() {
    std::stringstream ss;
    ss << "Name:\tcom.android.systemui\n";
    ss << "State:\tS (sleeping)\n";
    ss << "Tgid:\t" << (1000 + Crypto::SecureRandomGenerator().generateUint32() % 100) << "\n";
    ss << "Pid:\t" << (1000 + Crypto::SecureRandomGenerator().generateUint32() % 100) << "\n";
    ss << "PPid:\t1\n";
    ss << "TracerPid:\t0\n";
    ss << "Uid:\t1000\t1000\t1000\t1000\n";
    ss << "Gid:\t1000\t1000\t1000\t1000\n";
    ss << "FDSize:\t512\n";
    ss << "Groups:\t1004 1007 1011 1015 1028 1032 2000 3001 3002 3003 3006 3007 3009 3010 99998 99999\n";
    ss << "CapInh:\t0000000000000000\n";
    ss << "CapEff:\t0000000000000000\n";
    ss << "CapBnd:\t000000ffffffffff\n";
    ss << "CapAmb:\t0000000000000000\n";
    ss << "NoNewPrivs:\t0\n";
    ss << "Seccomp:\t2\n";
    ss << "Speculation_Store_Bypass:\tunknown\n";
    ss << "Cpus_allowed:\tff\n";
    ss << "Cpus_allowed_list:\t0-7\n";
    ss << "Mems_allowed:\t00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000000,00000001\n";
    ss << "Mems_allowed_list:\t0\n";
    ss << "voluntary_ctxt_switches:\t" << (10000 + Crypto::SecureRandomGenerator().generateUint32() % 100000) << "\n";
    ss << "nonvoluntary_ctxt_switches:\t" << (500 + Crypto::SecureRandomGenerator().generateUint32() % 10000) << "\n";
    return ss.str();
}

std::string ProcEmulator::getCmdLine() {
    return "surfaceflinger\0";
}

std::string ProcEmulator::getBatteryStatus() {
    std::stringstream ss;
    ss << "Battery.present: 1\n";
    ss << "Battery.status: 2\n";
    ss << "Battery.health: 2\n";
    ss << "Battery.level: 75\n";
    ss << "Battery.counter_type: 4\n";
    ss << "Battery.current_avg: -350\n";
    ss << "Battery.current_now: -380\n";
    ss << "Battery.voltage_now: 4200000\n";
    ss << "Battery.temperature: 285\n";
    return ss.str();
}

std::string ProcEmulator::getThermalStatus() {
    return "0\n"; // Normal thermal zone
}

std::string ProcEmulator::generateRandomHex(int bytes) {
    return Crypto::SecureRandomGenerator().generateHexString(bytes * 2);
}

std::string ProcEmulator::generateCpuImplementer() {
    return m_cpuConfig.implementer;
}

std::string ProcEmulator::generateCpuVariant() {
    Crypto::SecureRandomGenerator rng;
    char buf[5];
    snprintf(buf, sizeof(buf), "0x%02x", (unsigned int)(0x10 + rng.generateUint32() % 0xF0));
    return buf;
}

std::string ProcEmulator::generateCpuPart() {
    Crypto::SecureRandomGenerator rng;
    char buf[6];
    snprintf(buf, sizeof(buf), "0x%03x", (unsigned int)(0x0D05 + rng.generateUint32() % 0x100));
    return buf;
}

std::string ProcEmulator::generateCpuRevision() {
    Crypto::SecureRandomGenerator rng;
    return std::to_string(rng.generateUint32() % 10);
}

std::map<std::string, std::string> ProcEmulator::getAllProcValues() {
    std::map<std::string, std::string> values;
    values["/proc/cpuinfo"] = getCpuInfo();
    values["/proc/version"] = getVersion();
    values["/proc/uptime"] = getUptime();
    values["/proc/meminfo"] = getMemInfo();
    values["/proc/self/mounts"] = getMounts();
    values["/proc/self/status"] = getStatus();
    values["/sys/class/power_supply/battery/status"] = getBatteryStatus();
    values["/sys/class/thermal/thermal_zone0/trip_point_0_temp"] = getThermalStatus();
    return values;
}

} // namespace VirtualPhonePro
