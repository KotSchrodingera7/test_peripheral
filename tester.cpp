

#include "tester.h"

#include <QFileInfo>
#include <QProcess>
#include <QNetworkInterface>
#include <QThread>
#include <QDateTime>
#include <QtConcurrent/QtConcurrent>

// #include "face3drfid.h"
// #include "face3dwiegand.h"
#include "can_thread.h"
#include "spi_test.h"
#include "test_uart.h"
#include "logger.h"

#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>
#include <ctime>
#include <sstream>
#include <fstream>
#include "tester_debug.h"

//#define DEBUG
#define MAX_BUF 100
#define BUF_SIZ 255
#define CONFIG "/usr/local/biosmart/quasar-test.conf"
#define LOG "/usr/local/biosmart/quasar-test.log"

bool verbose_logs = false;

Q_LOGGING_CATEGORY(c_test, "TESTER")
Q_LOGGING_CATEGORY(c_sd, "uSD")
Q_LOGGING_CATEGORY(c_usb2, "USB2")
Q_LOGGING_CATEGORY(c_usb3, "USB3")
Q_LOGGING_CATEGORY(c_gpio, "GPIO")
Q_LOGGING_CATEGORY(c_gpio1, "GPIO1")
Q_LOGGING_CATEGORY(c_gpio2, "GPIO2")
Q_LOGGING_CATEGORY(c_gpio3, "GPIO3")
Q_LOGGING_CATEGORY(c_eth1, "ETHERNET0")
Q_LOGGING_CATEGORY(c_eth2, "ETHERNET1")
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
    cmd.append(LOG);
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

void attention() {
    system("aplay /usr/local/biosmart/sounds/ok.wav > /dev/null 2>&1");
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
    usb2(Result::NotTested),
    usb3(Result::NotTested),
    lsd(Result::NotTested),
    touchscreen(Result::NotTested),
    gpio(Result::NotTested),
    ethernet1(Result::NotTested),
    ethernet2(Result::NotTested),
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

int Tester::testMicrosd()
{
    bool result = false;
    QString name = "microsd";
    singleTestResult(name, Result::Progress);

    QString usd_path = "/dev/mmcblk0";
    QFile handler(usd_path);
    result = handler.exists();

    Result res = SetResAddedLogs(c_sd, result, "TEST Success", "Not find device of mmcblk0");


    singleTestResult(name, res);

    return res;
}

int Tester::testUsb2()
{
    bool result = false;
    QString name = "usb2";
    singleTestResult(name, Result::Progress);

    QString str_res_first;
    QString str_res_second;
    QString str_res_third;

    bool check_folder = (bool)system("test -d /media/rootfs");

    if( check_folder )
    {
        system("mkdir /media/rootfs");
    }

    result = !(bool)system("mount /dev/sda1 /media/rootfs");
    Result res = SetResAddedLogs(c_usb2, result, "TEST Success", "Not mount file system");

    system("umount /media/rootfs");
    singleTestResult(name, res);

    return res;
}

int Tester::testSpeaker()
{
    QtConcurrent::run([=](){
        bool result = false;
        QString name = "speaker";
//        singleTestResult(name, Result::Progress);

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
        QString name = "speaker";
//        singleTestResult(name, Result::Progress);

        usleep(50000);
        result = !(bool)(system("gst-launch-1.0 -v v4l2src device=/dev/video0 ! video/x-raw,format=NV12,width=1024,height=600,framerate=30/1 ! videoconvert ! waylandsink > /dev/null 2>&1 &"));
        sleep(10);
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

    if( system("ip link set can0 type can bitrate 125000 triple-sampling on > /dev/null 2>&1") ) {
        qCWarning(c_can0) << "Set bitrate with triple-sampling error";
    }
    usleep(50000);
    if ( system("ip link set can1 type can bitrate 125000 triple-sampling on > /dev/null 2>&1") ) {
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

    
    Result res = SetResAddedLogs(c_can0, result, "TEST Success", "Read data and write data don't equal");
    singleTestResult(name, res);

    return res;
}
int Tester::testSpi1()
{
    bool result = false;
    QString name = "spi1";

    SpiTest spidev_("/dev/spidev1.0");

    if( spidev_.TestTransfer() == 0 )
    {
        result = true;
    }

    Result res = SetResAddedLogs(c_spi1, result);
    singleTestResult(name, res);

    return res;
}

int Tester::testSpi2()
{
    bool result = false;
    QString name = "spi2";

    SpiTest spidev_("/dev/spidev2.0");


    if( spidev_.TestTransfer() == 0 )
    {
        result = true;
    }

    Result res = SetResAddedLogs(c_spi2, result);
    singleTestResult(name, res);


    return res;
}

int Tester::testNvme()
{
    bool result = false;
    QString name = "nvme";
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

int Tester::testEthernet1()
{
    bool result = false;
    QString name = "ethernet1";
    singleTestResult(name, Result::Progress);

    QString cmd = "ping -I end0 -c 1 -W 2 ";
    cmd.append("8.8.8.8");
    cmd.append(" 1>/dev/null 2>/dev/null");
    int returned = system(qPrintable(cmd));

    result = (returned == 0);
    if (verbose_logs) {
        std::cout << "========================================" << std::endl;
        std::cout << "Test: " << name.toStdString() << std::endl;
        std::cout << "target ip: " << testTargetIp.toStdString() << std::endl;
        std::cout << "returned code: " << returned << std::endl;
        std::cout << std::endl;
    }

    usleep(50000);
    Result res = SetResAddedLogs(c_eth1, result);
    singleTestResult(name, res);
    
    return res;
}

int Tester::testEthernet2()
{
    bool result = false;
    QString name = "ethernet2";
    singleTestResult(name, Result::Progress);

    QString cmd = "ping -I end1 -c 1 -W 2 ";
    cmd.append("8.8.8.8");
    cmd.append(" 1>/dev/null 2>/dev/null");
    int returned = system(qPrintable(cmd));

    result = (returned == 0);
    if (verbose_logs) {
        std::cout << "========================================" << std::endl;
        std::cout << "Test: " << name.toStdString() << std::endl;
        std::cout << "target ip: " << testTargetIp.toStdString() << std::endl;
        std::cout << "returned code: " << returned << std::endl;
        std::cout << std::endl;
    }

    usleep(50000);
    Result res = SetResAddedLogs(c_eth2, result);
    singleTestResult(name, res);

    
    return res;
}

int Tester::init()
{
    bool result = true;

    qCInfo(c_test);
    qCInfo(c_test);
    qCInfo(c_test) << "Quasar motherboard testing software";
    qCInfo(c_test) << "========================================";
    qCInfo(c_test);
    qCInfo(c_test) << "initialization started";
    qCInfo(c_test) << "----------------------------------------";

    qCInfo(c_test);
    qCInfo(c_test) << "getting device ip";
    qCInfo(c_test) << "----------------------------------------";
    ip = get_current_ip();

    if (verbose_logs) {
        // qCInfo(c_test) << "test started at: " << QDateTime::currentDateTime().toString().toStdString() << std::endl;
        QString cmd = "echo \'Test started: ";
        cmd.append(QDateTime::currentDateTime().toString());
        cmd.append("\n\'");
        cmd.append(" > ");
        cmd.append(LOG);
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
    };

    return (result) ? Result::Success : Result::Failed;
}

int Tester::test()
{
    QDateTime now;
    results.date = now.date().toString();

    results.usb2 = static_cast<Result>(testUsb2());
    results.microsd = static_cast<Result>(testMicrosd());
    // attention();
    results.gpio = static_cast<Result>(testGPIO());
    results.ethernet1 = static_cast<Result>(testEthernet1());
    results.ethernet2 = static_cast<Result>(testEthernet2());
    results.can = static_cast<Result>(testCan());
    results.spi1 = static_cast<Result>(testSpi1());
    results.spi2 = static_cast<Result>(testSpi2());
    results.nvme = static_cast<Result>(testNvme());
    results.uart78 = static_cast<Result>(testUart78());
    results.uart39 = static_cast<Result>(testUart39());
    results.wlan = static_cast<Result>(testWlan());
    

    //    results.usb3 = static_cast<Result>(testUsb3());
    //    results.lsd = static_cast<Result>(testLsd());
    //    results.touchscreen = static_cast<Result>(testTouchscreen());
    //    results.speaker = static_cast<Result>(testSpeaker());
    //    results.led = static_cast<Result>(testLed());


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
            std::cout << "ifstream not open" << std::endl;
            return value;
        }
    } else
    {
        std::cout << "Do not access to gpio" << std::endl;
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
    if( init_gpio(in, std::string("in"), 0) ;
    init_gpio(out, std::string("out"), 1);
    set_value(out, 1);

    uint32_t value = get_value(in);

    std::cout << "GPIO value in " << value << std::endl;
    set_value(out, 0);
    
    value &= (!get_value(in));

    std::cout << "GPIO value in after set" << value << std::endl;

    return value;
}

int Tester::testGPIO()
{
    bool result = false;
    QString name = "gpio";

    uint32_t value = gpio_pair_check(GpioDefinition::GPIO2, GpioDefinition::GPIO3);
    value &= gpio_pair_check(GpioDefinition::GPIO3, GpioDefinition::GPIO2);
    value &= gpio_pair_check(GpioDefinition::GPIO0, GpioDefinition::GPIO1);
    value &= gpio_pair_check(GpioDefinition::GPIO1, GpioDefinition::GPIO0);
    value &= gpio_pair_check(GpioDefinition::UART7_CTS, GpioDefinition::UART8_RTS);
    value &= gpio_pair_check(GpioDefinition::UART8_CTS, GpioDefinition::UART7_RTS);
    value &= gpio_pair_check(GpioDefinition::UART3_CTS, GpioDefinition::SPI1_CS0);

    Result res = (value) ? Result::Success : Result::Failed;
    singleTestResult(name, res);

    

    return res;
}

int Tester::testGpio1()
{
    bool result = false;
    QString name = "gpio1";

    uint32_t value = gpio_pair_check(35, 93);

    Result res = (value) ? Result::Success : Result::Failed;
    singleTestResult(name, res);

    

    return res;
}

int Tester::testGpio2()
{
    bool result = false;
    QString name = "gpio2";

    uint32_t value = gpio_pair_check(35, 92);
    std::cout << "Value in 35 out 92 = " << value << std::endl;

    Result res = (value) ? Result::Success : Result::Failed;
    singleTestResult(name, res);

    

    return res;
}

int Tester::testGpio3()
{
    bool result = false;
    QString name = "gpio3";

    uint32_t value = gpio_pair_check(35, 34);
    std::cout << "Value in 35 out 34 = " << value << std::endl;

    Result res = (value) ? Result::Success : Result::Failed;
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
    res["usb2"] = results.usb2;
    res["usb3"] = results.usb3;
    res["lsd"] = results.lsd;
    res["touchscreen"] = results.touchscreen;
    res["gpio"] = results.gpio;
    res["ethernet1"] = results.ethernet1;
    res["ethernet2"] = results.ethernet2;
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

// bool SerialReader::Init(QString file)
// {
//     setPortName(file.toUtf8());
//     setBaudRate(QSerialPort::Baud115200);
//     setFlowControl(QSerialPort::SoftwareControl);

//     if (!open(QIODevice::ReadWrite)) {
//         std::cout << "serial port open failed: " << file.toStdString() << std::endl;
//         return false;
//     }
//     else {
//         return true;
//     }
// }

// void SerialReader::ReadSerial()
// {
//     if (!canReadLine()) {
//         std::cout << "not ready" << std::endl;
//         return;
//     }
//     data_ = QSerialPort::readAll();
// }

// QByteArray SerialReader::GetData()
// {
//     return data_;
// }
