#ifndef TESTER_H
#define TESTER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
// #include <QSerialPort>
#include <unordered_map>

class Tester : public QObject
{
    Q_OBJECT

    Q_PROPERTY(Tester* tester READ tester)
    Q_PROPERTY(QString addr READ addr)

public:

    enum Result {
        Success = 0,
        Failed = 1,
        Error = 2,
        Progress = 3,
        NotTested = 4,
        Manual = 5,
    };

    Q_ENUM(Result)

    struct TestResult {
        TestResult();
        QString date;

        int success;
        int failed;
        int error;

        Result microsd;
        Result emmc;
        Result usbc;
        Result usb3;
        Result lsd;
        Result touchscreen;
        Result gpio;
        Result gpio1;
        Result gpio2;
        Result gpio3;
        Result ethernet;
        Result pcie2;
        Result pcie3;
        Result sound;
        Result can;
        Result uart78;
        Result uart39;
        Result spi1;
        Result spi2;
        Result nvme;
        Result wlan;
    };

    enum class GpioDefinition {
        GPIO0 = 36,
        GPIO1 = 38,
        GPIO2 = 39,
        GPIO3 = 40,
        UART3_RTS = 34,
        UART3_CTS = 35,
        UART8_CTS = 74,
        UART8_RTS = 73,
        UART7_CTS = 82,
        UART7_RTS = 81,
        SPI1_CS0 = 97,
        SPI2_CS0 = 93,
        SPI2_CS1 = 92,
    };

private:
    bool CheckSpeedUsb(const QLoggingCategory &name(), QString cmd, int limit);
    bool CheckNumberUsb(int number);
    QVariantMap serializeResults();
    void addGpio(int pin);
    void addOutput(int pin);
    void singleTestResult(QString name, Result value);
    void requestTestAction(QString action);

    QString cardNumber() const;
    void setCardNumber(const QString &value);

    QString targetIp() const;
    void setTargetIp(const QString &value);

    int testUart(int uart1, int uart2, bool check);
    // int testGpioPair(int first, int second);

    uint32_t get_value(int gpio_number);
    uint32_t gpio_pair_check(GpioDefinition in, GpioDefinition out);

    std::string set_value(int gpio_number, uint32_t value);
    std::string init_gpio(int gpio_number, std::string direction, uint32_t initial_value);

    TestResult results;
    QString ip;
    QString testTargetIp;
    QString  testCardNumber;

    std::unordered_map<GpioDefinition, std::string> gpio_definition_;

public slots:
    void receiveAction(QString info);

public:
    explicit Tester(QObject *parent = nullptr);

    Tester* tester();
    QString addr();
    Result SetResAddedLogs(const QLoggingCategory &name(), bool result, QString success = "TEST Success", QString error = "TEST Failed");

    // Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    // Q_PROPERTY(QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
    Q_INVOKABLE int testEmmc();
    Q_INVOKABLE int testMicrosd();
    Q_INVOKABLE int testUsbC();
    Q_INVOKABLE int testUsb3();
    Q_INVOKABLE int testEthernet();
    Q_INVOKABLE int testCan();
    Q_INVOKABLE int testSpi1();
    Q_INVOKABLE int testSpi2();
    Q_INVOKABLE int testUart78();
    Q_INVOKABLE int testUart39();
    Q_INVOKABLE int testNvme();
    Q_INVOKABLE int testPcie();
    Q_INVOKABLE int testSpeaker();
    Q_INVOKABLE int testCamera();
    Q_INVOKABLE int testWlan();


    Q_INVOKABLE int init();
    Q_INVOKABLE int test();
    Q_INVOKABLE int testUSB();
    Q_INVOKABLE int testGPIO();

    // Q_INVOKABLE int testGpio1();
    // Q_INVOKABLE int testGpio2();
    // Q_INVOKABLE int testGpio3();

    Q_INVOKABLE void printResults();
    Q_INVOKABLE bool saveResults();

    Q_INVOKABLE void runTest(QString name);

signals:
    void testFinished(QVariant results);
    void singleTestFinished(QVariant results);
    void needAction(QVariant results);

    void cardNumberChanged(QString value);
    void targetIpChanged(QString value);

};

#endif // TESTER_H
