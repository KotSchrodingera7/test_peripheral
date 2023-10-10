import QtQuick 2.12
import QtQuick.Window 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import macro.tester 1.0

Window {
    property bool dark_mode: true
    property string background_color_light: "#969696"
    property string background_color_dark: "#363636"
    property string font_color: "#57DCBE"
    property string success_color: "#57DCBE"
    property string failed_color: "#E91E63"
    property string unknown_color: "#276BB0"
    property bool touch: touch
    property bool success: false
    property bool allow_error: true
    property bool camera_tested: false
    property bool speaker_tested: false

    visible: true
    width:600
    height:1024
    title: qsTr("DS-RK3568 test")
    color: (dark_mode) ? background_color_dark : background_color_light
    
    Column {
        id: tests_col
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 32

        Column {
            id: headerCol
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 8

            RowLayout {
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                spacing: 32
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter

                Label {
                    id: mainText

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 16
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: "DS-RK3568 testing:"
                }

                Label {
                    id: statusText

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 32
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: "not started"
                }
            }

            RowLayout {
                anchors.leftMargin: 32
                anchors.rightMargin: 16
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignJustify

                Label {
                    id: addrLabel

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: "Ip address: "
                }

                Label {
                    id: addrText

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: tester.addr
                }
            }

            RowLayout {
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignJustify

                Label {
                    id: targetIpLabel

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: "Ping target: "
                }

                Label {
                    id: targetIpText

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: tester.targetIp
                }
            }

            RowLayout {
                anchors.leftMargin: 32
                anchors.rightMargin: 32
                spacing: 16
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignJustify

                Label {
                    id: cardNumberLabel
                    visible: false

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: "Requested card: "
                }

                Label {
                    id: cardNumberText
                    visible: false

                    font.capitalization: Font.AllUppercase
                    font.pixelSize: 24
                    font.bold: true
                    font.family: "Inter"

                    color: font_color

                    text: tester.cardNumber
                }
            }
        }

        Column {
            id: contentCol
            anchors.left: parent.left
            
            spacing: 2
            TestRow {
                id: microsd_test
                title: "Micro SD"
                restart.onClicked: function () { setValue(tester.testMicrosd()) }
            }


            TestRow {
                enabled: false
                visible: false
                id: touchscreen_test
                title: "Touchscreen"
                restart.onClicked: function () { setValue(tester.testTouchscreen()) }
            }

            TestRow {
                id: gpio_test
                title: "GPIO"
                restart.onClicked: function () { setValue(tester.testGPIO()) }
            }

            TestRow {
                id: ethernet1_test
                title: "Ethernet1"
                restart.onClicked: function () { setValue(tester.testEthernet1()) }
            }

            TestRow {
                id: ethernet2_test
                title: "Ethernet2"
                restart.onClicked: function () { setValue(tester.testEthernet2()) }
            }

            TestRow {
                id: can_test
                title: "Can"
                restart.onClicked: function () { setValue(tester.testCan()) }
            }

            TestRow {
                id: usb2_test
                title: "USB 2.0"
                restart.onClicked: function () { setValue(Tester.Progress); setValue(tester.testUsb2()) }
            }

            TestRow {
                enabled: false
                visible: false
                id: usb3_test
                title: "USB 3.0"
                restart.onClicked: function () { setValue(tester.testUsb3()) }
            }

            TestRow {
                id: spi1_test
                title: "SPI1"
                restart.onClicked: function () { setValue(tester.testSpi1()) }
            }

            TestRow {
                id: spi2_test
                title: "SPI2"
                restart.onClicked: function () { setValue(tester.testSpi2()) }
            }

            TestRow {
                id: nvme_test
                title: "NVME"
                restart.onClicked: function () { setValue(tester.testNvme()) }
            }

            TestRow {
                id: wlan_test
                title: "WLAN"
                restart.onClicked: function () { setValue(tester.testWlan()) }
            }

            TestRow {
                id: uart78_test
                title: "UART7-8"
                restart.onClicked: function () { setValue(tester.testUart78()) }
            }

            TestRow {
                id: uart39_test
                title: "UART3-9"
                restart.onClicked: function () { setValue(tester.testUart39()) }
            }

            TestRow {
                id: gpio1_test
                title: "GPIO1"
                restart.onClicked: function () { setValue(tester.testGpio1()) }
            }

            TestRow {
                id: gpio2_test
                title: "GPIO2"
                restart.onClicked: function () { setValue(tester.testGpio2()) }
            }

            TestRow {
                id: gpio3_test
                title: "GPIO3"
                restart.onClicked: function () { setValue(tester.testGpio3()) }
            }

        }

        Column {
            id: information
            height: 100
            width: parent.width
            spacing: 32

            Row {
                id: colorsRow
                anchors.horizontalCenter : parent.horizontalCenter
                visible: false
                spacing: 32

                Rectangle {
                    height: 50
                    width: 100
                    color: "red"
                }

                Rectangle {
                    height: 50
                    width: 100
                    color: "blue"
                }

                Rectangle {
                    height: 50
                    width: 100
                    color: "green"
                }
            }

            Text {
                anchors.topMargin: 32
                id: autotest_text
                text: "Проверка начнется автоматически"
                font.bold: true
                font.family: "Inter"
                color: "white"
                font.pixelSize: 20
                anchors.horizontalCenter: parent.horizontalCenter
            }

//            Label {
//                id: buttonsText
//                anchors.horizontalCenter: parent.horizontalCenter
//                visible: false

//                padding: 8
//                Layout.fillWidth: true
//                Layout.alignment: Qt.AlignRight

//                font.capitalization: Font.AllUppercase
//                font.pixelSize: 28
//                font.bold: true
//                font.family: "Inter"

//                color: "white"

//                text: "Проверка"
//            }

            Row {
                id: buttonsRow
                anchors.horizontalCenter: parent.horizontalCenter
                visible: false
                spacing: 32

                Button {
                    id: speaker_button
                    width: 200
                    height: 50

                    background: Rectangle {
                        id: bg_speaker_button
                        anchors.fill: parent
                        color: "white"
                    }

                    text: "Звук"
                    onClicked: function() {
                        bg_speaker_button.color = font_color
                        tester.testSpeaker()
                        buttonsRow.enabled = false
                    }
                }

                Button {
                    id: camera_button
                    width: 200
                    height: 50

                    background: Rectangle {
                        id: bg_camera_button
                        anchors.fill: parent
                        color: "white"
                    }
                    text: "Камера"
                    onClicked: function() {
                        bg_camera_button.color = font_color
                        tester.testCamera()
                        buttonsRow.enabled = false
                    }
                }
            }

            Button {
                id: touchscreen_button
                width: 432
                height: 50
                anchors.horizontalCenter: parent.horizontalCenter
                visible: camera_tested && speaker_tested
                enabled: true

                text: "Завершить"
                onClicked: function() {
                    tester.testTouchscreen()
                    enabled = false
                }
            }
        }
    }

    Timer {
        id: autotest_timer
        running: true
        interval: 3000
        repeat: false
        onTriggered: function() {
            autotest_text.text = "Тест запущен"
            statusText.text = "Running"
            statusText.color = "white"
            tester.runTest("all")
        }
    }

    Connections {
        target: tester
        onTestFinished: fillTestResults(results)
        onSingleTestFinished: parseSingleTest(results)
        onNeedAction: showNeededAction(results)
    }

    function showNeededAction(results) {
        var data = results

        var action = data["action"]

        autotest_text.text = action
    }

    function fillTestResults(results) {
        var data = results

        microsd_test.setValue(data["microsd"])
        usb2_test.setValue(data["usb2"])
        usb3_test.setValue(data["usb3"])
        // lsd_test.setValue(data["lsd"])
        touchscreen_test.setValue(data["touchscreen"])
        // rfid_test.setValue(data["rfid"])
        // rs485_test.setValue(data["rs485"])
        // tmpr_test.setValue(data["tmpr"])
        // iprst_test.setValue(data["iprst"])
        // relay_test.setValue(data["relay"])
        gpio_test.setValue(data["gpio"])
        can_test.setValue(data["can"])
        // wiegand_test.setValue(data["wiegand"])
        // speaker_test.setValue(data["speaker"])
        // led_test.setValue(data["led"])
        // rtc_test.setValue(data["rtc"])
        // eeprom_test.setValue(data["eeprom"])
        ethernet1_test.setValue(data["ethernet1"])
        ethernet2_test.setValue(data["ethernet2"])
        spi1_test.setValue(data["spi1"])
        spi2_test.setValue(data["spi2"])
        nvme_test.setValue(data["nvme"])
        wlan_test.setValue(data["wlan"])
        uart78_test.setValue(data["uart78"])
        uart39_test.setValue(data["uart39"])

        gpio1_test.setValue(data["gpio1"])
        gpio2_test.setValue(data["gpio2"])
        gpio3_test.setValue(data["gpio3"])
        // tamper_test.setValue(data["tamper"])
        // usbhub_test.setValue(data["usbhub"])

   // usbhub_test.setValue(data["usbhub"])
        if ((data["failed"] === 0 && allow_error) || (data["error"] === 0 && data["failed"])) {
            success = true//            statusText.text = "Success"
    //            statusText.color = "green"
        } else {
            success = false
//            statusText.text = "Failed"
//            statusText.color = "red"
        }

        autotest_text.text = "Для повторной проверки\nподсветки, звука\nвоспользуйтесь кнопками"
        buttonsRow.visible = true
        colorsRow.visible = true
    }

    function parseSingleTest(results) {
        var data = results

        var name = data["name"]
        var value = data["value"]

        if (name === "microsd") {
            microsd_test.setValue(value)
        } else if (name === "usb2") {
            usb2_test.setValue(value)
        } else if (name === "usb3") {
            usb3_test.setValue(value)
        } else if (name === "lsd") {
            lsd_test.setValue(value)
        } else if (name === "touchscreen") {
//            touchscreen_button.enabled = true
            touchscreen_test.setValue(value)

            if (success) {
                statusText.text = "Success"
                statusText.color = "green"
            } else {
                statusText.text = "Failed"
                statusText.color = "red"
            }
        } else if (name === "rfid") {
            rfid_test.setValue(value)
        } else if (name === "rs485") {
            rs485_test.setValue(value)
        } else if (name === "tmpr") {
            tmpr_test.setValue(value)
        } else if (name === "iprst") {
            iprst_test.setValue(value)
        } else if (name === "relay") {
            relay_test.setValue(value)
        } else if (name === "gpio") {
            gpio_test.setValue(value)
        } else if (name === "can") {
            can_test.setValue(value)
        } else if (name === "speaker") {
            bg_speaker_button.color = "white"
            buttonsRow.enabled = true
            speaker_tested = true
//            touchscreen_button.visible = camera_tested && speaker_tested
        } else if (name === "camera") {
            bg_camera_button.color = "white"
            camera_tested = true
            buttonsRow.enabled = true
//            touchscreen_button.visible = camera_tested && speaker_tested
        } else if (name === "rtc") {
            rtc_test.setValue(value)
        } else if (name === "eeprom") {
            eeprom_test.setValue(value)
        } else if (name === "ethernet1") {
            ethernet1_test.setValue(value)
        } else if (name === "ethernet2") {
            ethernet2_test.setValue(value)
        } else if (name === "tamper") {
            tamper_test.setValue(value)
        } else if (name === "usbhub") {
            usbhub_test.setValue(value)
        } else if (name === "spi1") {
            spi1_test.setValue(value)
        } else if (name === "spi2") {
            spi2_test.setValue(value)
        } else if( name == "nvme" ) {
            nvme_test.setValue(value)
        } else if( name == "uart78" ) {
            uart78_test.setValue(value)
        } else if( name == "uart39" ) {
            uart39_test.setValue(value)
        } else if( name == "gpio1" ) {
            gpio1_test.setValue(value)
        } else if( name == "gpio2" ) {
            gpio2_test.setValue(value)
        } else if( name == "gpio3" ) {
            gpio3_test.setValue(value)
        } else if( name == "wlan" ) {
            wlan_test.setValue(value)
        }
    }
}
