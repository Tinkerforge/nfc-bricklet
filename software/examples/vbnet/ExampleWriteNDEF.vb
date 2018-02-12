Imports System
Imports Tinkerforge

Module ExampleWriteNDEF
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your NFC Bricklet
    Const NDEF_URI As String = "www.tinkerforge.com"

    ' Callback subroutine for cardemu state changed callback
    Sub CardemuStateChangedCB(ByVal sender As BrickletNFC, ByVal state As Byte, _
                              ByVal idle As Boolean)
        If state = BrickletNFC.CARDEMU_STATE_IDLE Then
            Dim i As Integer
            Dim NDEFRecordURI(NDEF_URI.Length + 5) As Byte

            NDEFRecordURI(0) = &HD1
            NDEFRecordURI(1) = &H01
            NDEFRecordURI(2) = Convert.ToByte(NDEF_URI.Length + 1)
            NDEFRecordURI(3) = Asc("U")
            NDEFRecordURI(4) = &H04

            ' Only short records are supported
            For i = 0 To NDEF_URI.Length - 1
                NDEFRecordURI(5 + i) = Asc(NDEF_URI(i))
            Next

            For i = 0 To NDEFRecordURI.Length - 1
                Console.WriteLine(NDEFRecordURI(i))
            Next

            sender.CardemuWriteNdef(NDEFRecordURI)
            sender.CardemuStartDiscovery()
        ElseIf state = BrickletNFC.CARDEMU_STATE_DISCOVER_READY Then
            sender.CardemuStartTransfer(1)
        ElseIf state = BrickletNFC.CARDEMU_STATE_DISCOVER_ERROR Then
            Console.WriteLine("Discover error")
        ElseIf state = BrickletNFC.CARDEMU_STATE_TRANSFER_NDEF_ERROR Then
            Console.WriteLine("Transfer NDEF error")
        End If
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim nfc As New BrickletNFC(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register cardemu state changed callback to subroutine CardemuStateChangedCB
        AddHandler nfc.CardemuStateChangedCallback, AddressOf CardemuStateChangedCB

        ' Enable cardemu mode
        nfc.SetMode(BrickletNFC.MODE_CARDEMU)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
