

#include "tester.h"

#include <QFileInfo>
#include <QProcess>
#include <QNetworkInterface>
#include <QThread>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

#include "can_thread.h"
#include "test_uart.h"
#include "logger.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <string_view>
#include <ctime>
#include <sstream>
#include <istream>
#include <fstream>
#include "tester_debug.h"

//#define DEBUG
#define MAX_BUF 100
#define BUF_SIZ 255

bool verbose_logs = false;


static const std::map<std::string, std::string> map_cmd_fio {
    {"usb", "fio -loops=1 --size=100m --filename=/media/fatfs/fiotest.tmp --stonewall --ioengine=libaio --direct=1 --name=Seqread --bs=8M --rw=read > speed"},
    {"mmc", "fio -loops=1 --size=100m --filename=/root/fiotest.tmp --stonewall --ioengine=libaio --direct=1 --name=Seqread --bs=8M --rw=read > speed"}
};

Q_LOGGING_CATEGORY(c_test, "TESTER")
Q_LOGGING_CATEGORY(c_sd, "uSD")
Q_LOGGING_CATEGORY(c_emmc, "EMMC")
Q_LOGGING_CATEGORY(c_usbc, "USB-C")
Q_LOGGING_CATEGORY(c_usb3, "USB3")
Q_LOGGING_CATEGORY(c_gpio, "GPIO")
Q_LOGGING_CATEGORY(c_gpio1, "GPIO1")
Q_LOGGING_CATEGORY(c_gpio2, "GPIO2")
Q_LOGGING_CATEGORY(c_gpio3, "GPIO3")
Q_LOGGING_CATEGORY(c_eth, "ETHERNET")
Q_LOGGING_CATEGORY(c_can, "CAN SYSTEM")
Q_LOGGING_CATEGORY(c_can0, "CAN0")
Q_LOGGING_CATEGORY(c_can1, "CAN1")
Q_LOGGING_CATEGORY(c_uart, "UART system")
Q_LOGGING_CATEGORY(c_uart7, "UART7")
Q_LOGGING_CATEGORY(c_uart8, "UART8")
Q_LOGGING_CATEGORY(c_uart3, "UART3")
Q_LOGGING_CATEGORY(c_uart9, "UART9")
Q_LOGGING_CATEGORY(c_spi, "SPI system")
Q_LOGGING_CATEGORY(c_spi1, "SPI1")
Q_LOGGING_CATEGORY(c_spi2, "SPI2")
Q_LOGGING_CATEGORY(c_nvme, "NVME")
Q_LOGGING_CATEGORY(c_wlan, "WLAN")
Q_LOGGING_CATEGORY(c_camera, "CAMERA")
Q_LOGGING_CATEGORY(c_speaker, "SPEAKER")



Tester::Result Tester::SetResAddedLogs(const QLoggingCategory &name(), bool result, QString success, QString error) {
    Result res = Result::Failed;

    if( result ) {
        res = Result::Success;
        qCInfo(name) << success;
    } else {
        res = Result::Failed;
        qCCritical(name) << error;
    }


    return res;
}

void write_log(QString &data) {
    QString cmd;
    cmd.append("echo \'");
    cmd.append(data);
    cmd.append("\' >> ");
    system(qPrintable(cmd));
}

QString get_current_ip() {
    QString res = "unknown ip";
    QString cmd = "ifconfig end0 | grep \'inet addr:\' > current_ip";
    QString ans;
    QFile cur("current_ip");

    int sys = system(qPrintable(cmd));
    if (sys) {
        return res;
    }

    if (cur.open(QIODevice::ReadOnly)) {
        QByteArray data = cur.readAll();
        res = QString(data);
        if (res.contains("inet addr:")) {
            res = res.replace("inet addr:", "");
            res = res.remove(QChar(' '));
            for (QChar c : res) {
                if (c == 'B') {
                    break;
                } else {
                    ans.append(c);
                }
            }
            res = ans;
        }
    }

    return res;
}

QString default_ip_target() {
    QHostAddress address = QNetworkInterface::interfaceFromName("eth0").addressEntries().at(0).ip();
    QString ip = address.toString();
    if (address.isNull()) {
        ip = "172.25.110.254";
    } else {
        int pos = ip.lastIndexOf(QChar('.'));
        ip = ip.left(pos + 1);
        ip.append(QString::number(254));
    }

    return ip;
}

int run_cmd(const char *cmd, char lines[][BUF_SIZ]) {
    FILE *fp;
    char path[BUF_SIZ];

    fp = popen(cmd, "r");
    if (fp == NULL) {
        return -1;
    }

    int cnt = 0;
    while (fgets(path, sizeof(path), fp) != NULL) {
        strcpy(lines[cnt++], path);
    }
    pclose(fp);

    return cnt;
}

QStringList q_run_cmd(QString cmd) {
    char output[MAX_BUF][BUF_SIZ];
    const char *c_cmd = cmd.toStdString().c_str();
    int res = run_cmd(c_cmd, output);
    QStringList string_output;
    for (int i = 0; i < res; ++i) {
        QString line = QString::fromLocal8Bit(output[i]);
        string_output.push_back(line);
    }

    return string_output;
}

QString line_cmd(QString cmd) {
    QStringList list = q_run_cmd(cmd);
    QString out;
    if (list.size() > 0) {
       out = list.at(0);
    } else {
        std::cout << "no output for command: " << cmd.toStdString() << std::endl;
    }

    return out;
}

Tester::Tester(QObject *parent) : QObject(parent)
{
}

Tester::TestResult::TestResult() :
    microsd(Result::NotTested),
    usbc(Result::NotTested),
    usb3(Result::NotTested),
    lsd(Result::NotTested),
    touchscreen(Result::NotTested),
    gpio(Result::NotTested),
    ethernet(Result::NotTested),
    pcie2(Result::NotTested),
    pcie3(Result::NotTested),
    sound(Result::NotTested),
    can(Result::NotTested),
    uart78(Result::NotTested),
    uart39(Result::NotTested),
    spi1(Result::NotTested),
    spi2(Result::NotTested),
    nvme(Result::NotTested),
    gpio1(Result::NotTested),
    gpio2(Result::NotTested),
    gpio3(Result::NotTested),
    wlan(Result::NotTested)
{

}

void Tester::receiveAction(QString info)
{
    if (verbose_logs) {
        std::cout << "tests_action: " << info.toStdString() << std::endl;
    }
    if (info.contains("touched")) {
        results.touchscreen = Result::Success;
    }

    singleTestResult(info, Result::Success);
}

void Tester::runTest(QString name)
{
    if (verbose_logs) {
        std::cout << "tests_starting" << std::endl;
    }
    if (name == "all") {
        QtConcurrent::run([=](){
            test();
            if (verbose_logs) {
                std::cout << "tests_finished" << std::endl;
            }
        });
    }
    if (verbose_logs) {
        std::cout << "tests_started" << std::endl;
    }
}

QString Tester::addr()
{
    return ip;
}

void Tester::addGpio(int pin) {
    if (QFile::exists(QString("/sys/class/gpio/gpio%1/value").arg(QString::number(pin)))) {
        QString cmd = QString("echo %1 > /sys/class/gpio/unexport").arg(QString::number(pin));
        system(qPrintable(cmd));
    }
    QString cmd = QString("echo %1 > /sys/class/gpio/export").arg(QString::number(pin));
    system(qPrintable(cmd));
}

void Tester::addOutput(int pin)
{
    addGpio(pin);
    QString cmd;
    cmd = QString("echo out > /sys/class/gpio/gpio%1/direction").arg(QString::number(pin));
    system(qPrintable(cmd));
    cmd = QString("echo falling > /sys/class/gpio/gpio%1/edge").arg(QString::number(pin));
    system(qPrintable(cmd));
}

Tester* Tester::tester()
{
    return static_cast<Tester*>(this);
}

bool Tester::CheckSpeedUsb(const QLoggingCategory &name(), QString cmd, int limit) {
    bool result = false;
    QFile cur("speed");
    int sys = system(qPrintable(cmd));
    if( !sys ) {
        qInfo(name) << "Cmd OK";
        if (cur.open(QIODevice::ReadOnly)) {
            qInfo(name) << "Open file okay";
            QByteArray data = cur.readAll();

            std::istringstream input(QString(data).toStdString());

            for (std::string line; getline(input, line);) {
                const std::string find_str = "READ: bw=";
                size_t pos = line.find(find_str);
                if( pos != std::string::npos ) {
                    std::string str_(line.substr(pos + find_str.size()));
                    double speed = std::atof(str_.data());
                    qInfo(name) << "Speed = " << speed;

                    if( speed > limit ) {
                        result = true;
                    }
                }
            }
        }
    } else {
        qInfo(name) << "Cmd isn't ok" << cmd;
    }
    cur.remove("speed");
    return result;
}

int Tester::testEmmc()
{
    bool result = false;
    QString name = "emmc";
    singleTestResult(name, Result::Progress);

    QString emmc_path = "/dev/mmcblk0";
    QFile handler(emmc_path);
    if( handler.exists() ) {
        if( QFile("/dev/mmcblk0p3").exists() ) {
            result = CheckSpeedUsb(c_emmc, QString(map_cmd_fio.at("mmc").c_str()), 60);
        } else {
            QFile type_system("check_filesystem");

            int sys = system("fsck -N /dev/mmcblk0 | awk 'NR==2 {print $5}' > check_filesystem");

            if ( !sys ) {
                if (type_system.open(QIODevice::ReadOnly)) {
                    qInfo(c_emmc) << "Open file okay";
                    std::string str_ = type_system.readAll().toStdString();

                    if( str_ != "fsck.ext4\n" ) {
                        system("mkfs.ext4 /dev/mmcblk0");
                    }
                    if( !QFile("/media/fatfs").exists() ) {
                        system("mkdir /media/fatfs");
                    }
                    system("mount /dev/mmcblk0 /media/fatfs/ > /dev/null 2>&1");
                    result = CheckSpeedUsb(c_emmc, QString(map_cmd_fio.at("usb").c_str()), 60);
                } else {
                    qInfo(c_emmc) << "Can't open file";
                }

            }
            type_system.remove();
        }
    }
    
    Result res = SetResAddedLogs(c_emmc, result, "TEST Success", "Not find device of mmcblk0");
    singleTestResult(name, res);

    return res;
}

int Tester::testMicrosd()
{
    bool result = false;
    QString name = "microsd";
    singleTestResult(name, Result::Progress);

    QString usd_path = "/dev/mmcblk1";
    QFile handler(usd_path);
    if( handler.exists() ) {
        if( QFile("/dev/mmcblk1p3").exists() ) {
            result = CheckSpeedUsb(c_sd, QString(map_cmd_fio.at("mmc").c_str()), 10);
        }
    }

    Result res = SetResAddedLogs(c_sd, result, "TEST Success", "Not find device of mmcblk0");


    singleTestResult(name, res);

    return res;
}


bool Tester::CheckNumberUsb(int number) {
    QFile data("check_usb");
    int sys = system("lsusb > check_usb");

    if( !sys ) {
        if (data.open(QIODevice::ReadOnly)) {

            std::istringstream input(QString(data.readAll()).toStdString());

            for (std::string line; getline(input, line);) {
                size_t pos = line.find(' ');
                if( pos != std::string::npos ) {
                    std::string str_(line.substr(pos + 1));
                    int number_bus = std::atoi(str_.c_str());

                    if( number_bus != number ) {
                        continue;
                    }

                    if( str_.find("Linux Foundation") != std::string::npos ) {
                        continue;
                    }

                    return true;
                }
            }
        }
    }

    return false;
}

int Tester::testUsbC() {
    bool result = false;
    QString name = "usbc";
    singleTestResult(name, Result::Progress);
    system("umount /media/fatfs > /dev/null 2>&1");
    if( CheckNumberUsb(2) ) {

        bool check_folder = (bool)system("test -d /media/fatfs");

        if( check_folder )
        {
            system("mkdir /media/fatfs");
        }

        QString cmd;
        QString usd_path = "/dev/sda1";
        QFile handler(usd_path);
        if( handler.exists() ) {
            cmd = "mount /dev/sda1 /media/fatfs/ > /dev/null 2>&1";
        } else if( QFile("/dev/sda").exists() ) {
            cmd = "mount /dev/sda /media/fatfs/ > /dev/null 2>&1";
        }
        
        int sys = system(qPrintable(cmd));
        if (sys) {
            qCCritical(c_usbc) << "SYSTEM MOUNT FAIL";
        } else {
            result = CheckSpeedUsb(c_usbc, QString(map_cmd_fio.at("usb").c_str()), 60);
            system("umount /media/fatfs > /dev/null 2>&1");
        }

    }

    Result res = SetResAddedLogs(c_usbc, result, "TEST Success", "Not mount file system");
    singleTestResult(name, res);

    return res;

}

int Tester::testUsb3() 
{
    bool result = false;
    QString name = "usb3";
    singleTestResult(name, Result::Progress);

    system("umount /media/fatfs > /dev/null 2>&1");
    if( CheckNumberUsb(2) ) {
        bool check_folder = (bool)system("test -d /media/fatfs");

        if( check_folder )
        {
            system("mkdir /media/fatfs3");
        }

        QString cmd;
        if( QFile("/dev/sda1").exists() ) {
            cmd = "mount /dev/sda1 /media/fatfs/ > /dev/null 2>&1";
        } else if( QFile("/dev/sda").exists() ) {
            cmd = "mount /dev/sda /media/fatfs/ > /dev/null 2>&1";
        }

        int sys = system(qPrintable(cmd));
        if (sys) {
            qCCritical(c_usb3) << "SYSTEM MOUNT FAIL";
        } else {
            result = CheckSpeedUsb(c_usbc, QString(map_cmd_fio.at("usb").c_str()), 60);
        }
    }

    Result res = SetResAddedLogs(c_usb3, result, "TEST Success", "Not mount file system");

    system("umount /media/fatfs > /dev/null 2>&1");
    singleTestResult(name, res);

    return res;
}

int Tester::testSpeaker()
{
    QtConcurrent::run([=](){
        bool result = false;
        QString name = "speaker";

        usleep(50000);
        result = !(bool)(system("aplay --device=hw:1,0 /usr/local/share/sample-3s.wav > /dev/null 2>&1"));

        Result res = SetResAddedLogs(c_speaker, result, "System execuition success", "System execuition failed");

        singleTestResult(name, res);
    });

    return Result::Success;
}

int Tester::testCamera()
{
    QtConcurrent::run([=](){
        bool result = false;
        QString name = "camera";

        usleep(50000);
        result = !(bool)(system("gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1920,heigh=1080 ! v4l2convert ! video/x-raw,format=RGB16 ! waylandsink > /dev/null 2>&1 &"));
        sleep(20);
        system("killall gst-launch-1.0");

        Result res = SetResAddedLogs(c_camera, result, "System execuition success", "System execuition failed");

        singleTestResult(name, res);
    });

    return Result::Success;
}

int Tester::testCan()
{
    
    bool result = false;
    QString name = "can";
    singleTestResult(name, Result::Progress);

    usleep(50000);
    if( system("ip link set can0 down > /dev/null 2>&1") ) {
        qCWarning(c_can0) << "Link didn't set to down";
    }
    usleep(50000);

    if( system("ip link set can1 down > /dev/null 2>&1") ) {
        qCWarning(c_can1) << "Link didn't set to down";
    }
    usleep(50000);

    if ( system("ip link set can0 type can bitrate 1000000 > /dev/null 2>&1") ) {
        qCWarning(c_can0) << "Set bitrate with triple-sampling error";
    }
    usleep(50000);
    if ( system("ip link set can1 type can bitrate 1000000 > /dev/null 2>&1") ) {
        qCWarning(c_can1) << "Set bitrate with triple-sampling error";
    }
    usleep(50000);

    if( system("ip link set can0 up > /dev/null 2>&1") ) {
        qCWarning(c_can0) << "Link didn't set to up";
    }
    usleep(50000);
    if( system("ip link set can1 up") ) {
        qCWarning(c_can1) << "Link didn't set to up";
    }
    usleep(50000);

    CanThread can0("can0");
    CanThread can1("can1");
    sleep(1);

    std::thread can_recv(&CanThread::ThreadReceive, &can0);

    std::vector<uint8_t> data_send{0x54, 0x55, 0x56, 0x57};

    can1.SendMessage({0x5A, 0x4, 0x0, data_send});
    can_recv.join();

    if( data_send == can0.GetMessage().data )
    {
        result = true;
    }

    
    Result res = SetResAddedLogs(c_can, result, "TEST Success", " TEST FAIL : Read data and write data don't equal");
    singleTestResult(name, res);

    return res;
}
int Tester::testSpi1()
{
    bool result = false;
    QString name = "spi1";
    singleTestResult(name, Result::Progress);

    uint32_t value = gpio_pair_check(GpioDefinition::SPI1_MISO, GpioDefinition::SPI1_MOSI);

    Result res = SetResAddedLogs(c_spi1, value);
    singleTestResult(name, res);
    return res;
}

int Tester::testSpi2()
{
    bool result = false;
    QString name = "spi2";
    singleTestResult(name, Result::Progress);

    uint32_t value = gpio_pair_check(GpioDefinition::SPI2_MISO, GpioDefinition::SPI2_MOSI);

    Result res = SetResAddedLogs(c_spi2, value);
    singleTestResult(name, res);
    return res;
}

int Tester::testNvme()
{
    bool result = false;
    QString name = "nvme";
    singleTestResult(name, Result::Progress);
    auto status = system("test_nvme.sh > /dev/null 2>&1");

    if( status == 0 )
    {
        result = true;
    } 
    Result res = SetResAddedLogs(c_nvme, result);
    singleTestResult(name, res);

    return res;
}

int Tester::testWlan()
{
    bool result = false;
    QString name = "wlan";
    singleTestResult(name, Result::Progress);
    auto status = system("test_wlan_small.sh > /dev/null 2>&1");

    if( status == 0 )
    {
        result = true;
    } 
    Result res = SetResAddedLogs(c_wlan, result);
    singleTestResult(name, res);

    return res;
}


int Tester::testUart(int uart1, int uart2, bool check)
{
    bool result = false;

    Uart uart_dev_1, uart_dev_2;

    int r_ = uart_dev_1.Open(uart1);
    if( r_ != 0 )
    {
        return result;
    }
    r_ = uart_dev_2.Open(uart2);

    if( r_ != 0 )
    {
        return result;
    }

    std::string read_data_1, read_data_2;

    std::string send_data_1 = std::string("Write data").append(std::to_string(uart1)).append("-->").append(std::to_string(uart2));
	std::string send_data_2 = std::string("Write data").append(std::to_string(uart2)).append("-->").append(std::to_string(uart1));


    uart_dev_1.Write(send_data_1);
	uart_dev_2.Read(read_data_2);

    if( read_data_2 == send_data_1 ) 
    {
        qCInfo(c_uart) << "Data send from " << uart_dev_1.GetDev().c_str() << " equal with read data of " << uart_dev_2.GetDev().c_str();

        uart_dev_2.Write(send_data_2);
	    uart_dev_1.Read(read_data_1);
        if( read_data_1 == send_data_2) 
        {
            qCInfo(c_uart) << "Data send from " << uart_dev_2.GetDev().c_str() << " equal with read data of " << uart_dev_1.GetDev().c_str();
		    result = true;
        } else 
        {
            qCritical(c_uart) << "Data send from " << uart_dev_2.GetDev().c_str() << " DON'T equal with read data of " << uart_dev_1.GetDev().c_str();
        }

    } else {
        qCritical(c_uart) << "Data send from " << uart_dev_1.GetDev().c_str() << " DON'T equal with read data of " << uart_dev_2.GetDev().c_str();
    }

    return result;
}

int Tester::testUart78()
{
    bool result = false;
    QString name = "uart78";
    singleTestResult(name, Result::Progress);

    if( testUart(7, 8, true) )
    {
        result = true;
    }
    Result res = SetResAddedLogs(c_uart, result);
    singleTestResult(name, res);

    return res;
}

int Tester::testUart39()
{
    bool result = false;
    QString name = "uart39";
    singleTestResult(name, Result::Progress);

    if( testUart(3, 9, false ) )
    {
        result = true;
    }
    Result res = SetResAddedLogs(c_uart, result);
    singleTestResult(name, res);

    return res;
}

int Tester::testPcie()
{
    bool result = true;
    QString name = "pcie";
    Result res = (result) ? Result::Success : Result::Failed;
    singleTestResult(name, res);

    return res;
}


inline void set_pwm(int val) {
    const QString pwm = "/sys/class/pwm/pwmchip3/pwm0/duty_cycle";
    QString cmd = "echo ";
    cmd.append(QString::number(val));
    cmd.append(" > ");
    cmd.append(pwm);
    system(qPrintable(cmd));
}

int Tester::testEthernet()
{
    bool result = false, sys = false;
    QString name = "ethernet";
    singleTestResult(name, Result::Progress);

    QFile cur("ethernet");

    system("ip netns add ns_server 1>/dev/null 2>/dev/null");
    system("ip netns add ns_client 1>/dev/null 2>/dev/null");
    system("ip link set end0 netns ns_server 1>/dev/null 2>/dev/null");
    system("ip netns exec ns_server ip addr add dev end0 192.168.1.198/24 1>/dev/null 2>/dev/null");
    system("ip netns exec ns_server ip link set dev end0 up 1>/dev/null 2>/dev/null");
    system("ip link set end1 netns ns_client 1>/dev/null 2>/dev/null");
    system("ip netns exec ns_client ip addr add dev end1 192.168.1.197/24 1>/dev/null 2>/dev/null");
    system("ip netns exec ns_client ip link set dev end1 up 1>/dev/null 2>/dev/null");
    system("ip netns exec ns_server iperf -s -B 192.168.1.198 1>/dev/null 2>/dev/null &");
    usleep(50000);

    sys = system("ip netns exec ns_client iperf -c 192.168.1.198 -B 192.168.1.197 > ethernet &");
    sleep(20);
    if( !sys ) {
        if (cur.open(QIODevice::ReadOnly)) {
            QFile speed_eth("speed_eth");
            system("awk 'NR==7 {print $7}' ethernet > speed_eth");

            if( speed_eth.open(QIODevice::ReadOnly) ) {
                QByteArray data = speed_eth.readAll();
                QString res = QString(data);
                if( res.toInt() > 80 ) {
                    result = true;
                }
            }   
        }
    }
    
    system("killall iperf");

    system("ip netns del ns_server");
    system("ip netns del ns_client");

    usleep(50000);
    Result res = SetResAddedLogs(c_eth, result);
    singleTestResult(name, res);
    return res;
}

int Tester::init()
{
    bool result = true;

    qCInfo(c_test);
    qCInfo(c_test);
    qCInfo(c_test) << "Diasom RK3568 testing software";
    qCInfo(c_test) << "========================================";
    qCInfo(c_test);
    qCInfo(c_test) << "initialization started";
    qCInfo(c_test) << "----------------------------------------";

    qCInfo(c_test);
    qCInfo(c_test) << "getting device ip";
    qCInfo(c_test) << "----------------------------------------";
    // ip = get_current_ip();

    if (verbose_logs) {
        // qCInfo(c_test) << "test started at: " << QDateTime::currentDateTime().toString().toStdString() << std::endl;
        QString cmd = "echo \'Test started: ";
        cmd.append(QDateTime::currentDateTime().toString());
        cmd.append("\n\'");
        cmd.append(" > ");
        system(qPrintable(cmd));
    }

    qCInfo(c_test);
    qCInfo(c_test) << "added gpio sysfs interfaces";
    qCInfo(c_test) << "----------------------------------------";

    qCInfo(c_test);
    qCInfo(c_test) << "initialization finished";
    qCInfo(c_test) << "----------------------------------------";


    gpio_definition_ = {

        {GpioDefinition::GPIO0, "GPIO0"},
        {GpioDefinition::GPIO1, "GPIO1"},
        {GpioDefinition::GPIO2, "GPIO2"},
        {GpioDefinition::GPIO3, "GPIO3"},
        {GpioDefinition::UART3_RTS, "UART3_RTS"},
        {GpioDefinition::UART3_CTS, "UART3_CTS"},
        {GpioDefinition::UART8_CTS, "UART8_CTS"},
        {GpioDefinition::UART8_RTS, "UART8_RTS"},
        {GpioDefinition::UART7_CTS, "UART7_CTS"},
        {GpioDefinition::UART7_RTS, "UART7_RTS"},
        {GpioDefinition::SPI1_CS0, "SPI1_CS0"},
        {GpioDefinition::SPI2_CS0, "SPI2_CS0"},
        {GpioDefinition::SPI2_CS1, "SPI2_CS1"},
        {GpioDefinition::SPI1_CLK, "SPI1_CLK"},
        {GpioDefinition::SPI1_MISO, "SPI1_MISO"},
        {GpioDefinition::SPI1_MOSI, "SPI1_MOSI"},
        {GpioDefinition::SPI2_CLK, "SPI2_CLK"},
        {GpioDefinition::SPI2_MISO, "SPI2_MISO"},
        {GpioDefinition::SPI2_MOSI, "SPI2_MOSI"},
    };

    return (result) ? Result::Success : Result::Failed;
}

int Tester::test()
{
    QDateTime now;
    results.date = now.date().toString();

    results.microsd = static_cast<Result>(testMicrosd());
    results.emmc = static_cast<Result>(testEmmc());
    results.gpio = static_cast<Result>(testGPIO());
    results.ethernet = static_cast<Result>(testEthernet());
    results.can = static_cast<Result>(testCan());
    results.usbc = static_cast<Result>(testUsbC());
    results.spi1 = static_cast<Result>(testSpi1());
    results.spi2 = static_cast<Result>(testSpi2());
    results.nvme = static_cast<Result>(testNvme());
    results.wlan = static_cast<Result>(testWlan());
    results.uart78 = static_cast<Result>(testUart78());
    results.uart39 = static_cast<Result>(testUart39());
    
    


    QVariantMap data = serializeResults();

    emit testFinished(QVariant(data));

    bool result = (results.failed == 0 && results.error == 0);

    return (result) ? Result::Success : Result::Failed;
}

int Tester::testUSB()
{
    bool result = true;

    return (result) ? Result::Success : Result::Failed;
}


std::string Tester::set_value(int gpio_number, uint32_t value)
{
    std::string gpio_value_path = std::string("/sys/class/gpio/gpio").append(std::to_string(gpio_number)).append("/value");
    if (access(gpio_value_path.c_str(), F_OK) != -1)
    {
        std::ofstream gpio_value_stream(gpio_value_path.c_str(), std::ofstream::trunc);
        if (gpio_value_stream)
        {
            gpio_value_stream << std::to_string(value);
            usleep(50000);
            return "";
        }
        else
        {
            return std::string("Error write ").append(std::to_string(value)).append(" to ").append(gpio_value_path);
        }
    }
    else
    {
        return std::string("Error open gpio: ").append(gpio_value_path);
    }
}

uint32_t Tester::get_value(int gpio_number) 
{
    uint32_t value = -1;

    std::string gpio_value_path = std::string("/sys/class/gpio/gpio").append(std::to_string(gpio_number)).append("/value");
    if (access(gpio_value_path.c_str(), F_OK) != -1)
    {
        std::ifstream gpio_value_stream(gpio_value_path.c_str(), std::ifstream::in);
        if (gpio_value_stream)
        {
            std::string string_value;

            gpio_value_stream >> string_value;
            return std::atoi(string_value.c_str());
        }
        {
            qCWarning(c_gpio) << "ifstream not open";
            return value;
        }
    } else
    {
        qCWarning(c_gpio) << "Do not access to gpio";
    }

    return value;
}

std::string Tester::init_gpio(int gpio_number, std::string direction, uint32_t initial_value)
{
    try
    {
        std::string gpio_value_path = std::string("/sys/class/gpio/gpio").append(std::to_string(gpio_number)).append("/value");
        // Export
        if (access(gpio_value_path.c_str(), F_OK) == -1)
        {
            std::ofstream gpio_export_stream(std::string("/sys/class/gpio/export").c_str(), std::ofstream::trunc);
            if (gpio_export_stream)
            {
                gpio_export_stream << std::to_string(gpio_number);
            }
            else
            {
                return std::string("Error write ").append(std::to_string(gpio_number)).append(" to ").append("/sys/class/gpio/export");
            }
        }
        if (access(gpio_value_path.c_str(), F_OK) == -1)
        {
            return std::string("Error open gpio: ").append(gpio_value_path);
        }
        // Direction
        std::string gpio_direction_path = std::string("/sys/class/gpio/gpio").append(std::to_string(gpio_number)).append("/direction");
        std::ofstream gpio_direction_stream(gpio_direction_path.c_str(), std::ofstream::trunc);
        if (gpio_direction_stream)
        {
            gpio_direction_stream << direction;
        }
        else
        {
            return std::string("Error write ").append(direction).append(" to ").append(gpio_direction_path);
        }
        // Initial value
        if (direction == "out")
        {
            if (set_value(gpio_number, initial_value).size() > 0)
            {
                return std::string("Error set initial value for").append(gpio_direction_path);
            }
        }

        return "";
    }
    catch (const std::exception &e)
    {
        return std::string(e.what());
    }
}

uint32_t Tester::gpio_pair_check(GpioDefinition in, GpioDefinition out)
{
    if( init_gpio(static_cast<int>(out), std::string("out"), 1).size() > 0 ) {
        qCInfo(c_gpio) << "Error init " << gpio_definition_[out].c_str() << " like output";
        qCCritical(c_gpio) << "Error init " << gpio_definition_[out].c_str() << " like output";
        return 0;
    }

    if( init_gpio(static_cast<int>(in), std::string("in"), 0).size() > 0 ) {
        qCInfo(c_gpio) << "Error init " << gpio_definition_[in].c_str() << " like input";
        qCCritical(c_gpio) << "Error init " << gpio_definition_[in].c_str() << " like input";
        return 0;
    }
    
    if ( set_value(static_cast<int>(out), 1).size() > 0 ) {
        qCCritical(c_gpio) << "Error set logical 1 out " << gpio_definition_[out].c_str();
    } else {
        qCInfo(c_gpio) << "Set logical 1 success out " << gpio_definition_[out].c_str();
    }

    uint32_t value = get_value(static_cast<int>(in));
    qCInfo(c_gpio) << "Getting value  " << value << " in " << gpio_definition_[in].c_str();
    set_value(static_cast<int>(out), 0);

    if ( set_value(static_cast<int>(out), 0).size() > 0 ) {
        qCCritical(c_gpio) << "Error set logical 0 out " << gpio_definition_[out].c_str();
    } else {
        qCInfo(c_gpio) << "Set logical 0 success out " << gpio_definition_[out].c_str();
    }
    
    uint32_t value_0 = get_value(static_cast<int>(in));
    value &= (!value_0);

    qCInfo(c_gpio) << "Getting value  " << value_0 << " in " << gpio_definition_[in].c_str();

    return value;
}

int Tester::testGPIO()
{
    bool result = false;
    QString name = "gpio";
    singleTestResult(name, Result::Progress);

    uint32_t value = gpio_pair_check(GpioDefinition::GPIO3, GpioDefinition::GPIO2);
    value &= gpio_pair_check(GpioDefinition::GPIO1, GpioDefinition::GPIO0);
    value &= gpio_pair_check(GpioDefinition::UART7_CTS, GpioDefinition::UART8_RTS);
    value &= gpio_pair_check(GpioDefinition::UART8_CTS, GpioDefinition::UART7_RTS);

    init_gpio(static_cast<int>(GpioDefinition::UART3_CTS), "in", 0);
    init_gpio(static_cast<int>(GpioDefinition::UART3_RTS), "out", 0);
    init_gpio(static_cast<int>(GpioDefinition::SPI2_CS0), "out", 0);
    init_gpio(static_cast<int>(GpioDefinition::SPI2_CS1), "out", 0);
    init_gpio(static_cast<int>(GpioDefinition::SPI1_CS0), "out", 0);
    init_gpio(static_cast<int>(GpioDefinition::SPI1_CLK), "out", 0);
    init_gpio(static_cast<int>(GpioDefinition::SPI2_CLK), "out", 0);

    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::UART3_RTS);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI2_CS0);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI2_CS1);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI1_CS0);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI2_CLK);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI1_CLK);

    Result res = SetResAddedLogs(c_gpio, value);
    singleTestResult(name, res);
    return res;
}

void Tester::printResults()
{
    QVariantMap result = serializeResults();

    emit testFinished(QVariant(result));
}

bool Tester::saveResults()
{
    QVariantMap result = serializeResults();

    return true;
}

QVariantMap Tester::serializeResults()
{
    results.success = 0;
    results.failed = 0;
    results.error = 0;

    QVariantMap res;

    res["date"] = results.date;

    res["microsd"] = results.microsd;
    res["emmc"] = results.emmc;
    res["usbc"] = results.usbc;
    res["usb3"] = results.usb3;
    res["lsd"] = results.lsd;
    res["touchscreen"] = results.touchscreen;
    res["gpio"] = results.gpio;
    res["ethernet"] = results.ethernet;
    res["pci2"] = results.pcie2;
    res["pci3"] = results.pcie3;
    res["sound"] = results.sound;
    res["can"] = results.can;
    res["uart78"] = results.uart78;
    res["uart39"] = results.uart39;
    res["spi1"] = results.spi1;
    res["spi2"] = results.spi2;
    res["nvme"] = results.nvme;
    res["wlan"] = results.wlan;
    

    for (auto val : res.values()) {
        if (val.toInt() == Result::Success) {
            results.success++;
        }
        if (val.toInt() == Result::Failed) {
            results.failed++;
        }
        if (val.toInt() == Result::Error) {
            results.error++;
        }
    }

    res["success"] = results.success;
    res["failed"] = results.failed;
    res["error"] = results.error;

    return res;
}

QString testValueToText(Tester::Result value) {
    QString text;
    if (value == Tester::Result::Success) {
        text = "success";
    } else if (value == Tester::Result::Failed) {
        text = "failed";
    } else if (value == Tester::Result::Progress) {
        text = "progress";
    } else if (value == Tester::Result::Error) {
        text = "error";
    } else if (value == Tester::Result::NotTested) {
        text = "not tested";
    } else if (value == Tester::Result::Manual) {
        text = "manual";
    } else {
        text = "unexpected";
    }

    return text;
}

void Tester::singleTestResult(QString name, Result value)
{
    if (verbose_logs) {
        QString str = QDateTime::currentDateTime().toString();
        str.prepend("[");
        str.append("] ");
        str.append(name);
        str.append("\t\t\t");
        str.append(testValueToText(value));
        write_log(str);
    }
    QVariantMap data;
    data["name"] = name;
    data["value"] = value;

    emit singleTestFinished(QVariant(data));
}

void Tester::requestTestAction(QString action)
{
    QVariantMap data;
    data["action"] = action;

    emit needAction(QVariant(data));
}