#ifndef TESTER_H
#define TESTER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantMap>
// #include <QSerialPort>

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
        Result usb2;
        Result usb3;
        Result lsd;
        Result touchscreen;
        Result gpio;
        Result ethernet1;
        Result ethernet2;
        Result pcie2;
        Result pcie3;
        Result sound;
        Result can;
        Result uart78;
        Result uart39;
        Result spi1;
        Result spi2;
        Result nvme;
    };

private:
    QVariantMap serializeResults();
    void addGpio(int pin);
    void addOutput(int pin);
    void singleTestResult(QString name, Result value);
    void requestTestAction(QString action);

    QString cardNumber() const;
    void setCardNumber(const QString &value);

    QString targetIp() const;
    void setTargetIp(const QString &value);

    int testUart(int uart1, int uart2);
    // int testGpioPair(int first, int second);

    uint32_t get_value(int gpio_number);
    uint32_t gpio_pair_check(int in, int out);

    std::string set_value(int gpio_number, uint32_t value);
    std::string init_gpio(int gpio_number, std::string direction, uint32_t initial_value);

    TestResult results;
    QString ip;
    QString testTargetIp;
    QString  testCardNumber;

public slots:
    void receiveAction(QString info);

public:
    explicit Tester(QObject *parent = nullptr);

    Tester* tester();
    QString addr();

    // Q_PROPERTY(QString cardNumber READ cardNumber WRITE setCardNumber NOTIFY cardNumberChanged)
    // Q_PROPERTY(QString targetIp READ targetIp WRITE setTargetIp NOTIFY targetIpChanged)
    Q_INVOKABLE int testMicrosd();
    Q_INVOKABLE int testUsb2();
    Q_INVOKABLE int testUsb3();
    Q_INVOKABLE int testLsd();
    Q_INVOKABLE int testTouchscreen();
    Q_INVOKABLE int testEthernet1();
    Q_INVOKABLE int testEthernet2();
    Q_INVOKABLE int testCan();
    Q_INVOKABLE int testSpi1();
    Q_INVOKABLE int testSpi2();
    Q_INVOKABLE int testUart78();
    Q_INVOKABLE int testUart39();
    Q_INVOKABLE int testNvme();
    Q_INVOKABLE int testPcie();
    Q_INVOKABLE int testSpeaker();
    Q_INVOKABLE int testCamera();
    

    Q_INVOKABLE int init();
    Q_INVOKABLE int test();
    Q_INVOKABLE int testUSB();
    Q_INVOKABLE int testGPIO();

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

// class SerialReader : public QSerialPort
// {
//     Q_OBJECT

// public:
//     SerialReader();
//     bool Init(QString file);
//     QByteArray GetData();

// public slots:
//     void ReadSerial();

// private:
//     QByteArray data_;
// };

#endif // TESTER_H
