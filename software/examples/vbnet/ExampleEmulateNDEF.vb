Imports System
Imports Tinkerforge

Module ExampleEmulateNDEF
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your NFC Bricklet
    Const NDEF_URI As String = "www.tinkerforge.com"

    ' Callback subroutine for cardemu state changed callback
    Sub CardemuStateChangedCB(ByVal sender As BrickletNFC, ByVal state As Byte, _
                              ByVal idle As Boolean)
        If state = BrickletNFC.CARDEMU_STATE_IDLE Then
            Dim i As Integer
            Dim ndefURIBytes As Byte() = Text.Encoding.ASCII.GetBytes(NDEF_URI)
            Dim ndefRecordURI(ndefURIBytes.Length + 5) As Byte

            ndefRecordURI(0) = &HD1
            ndefRecordURI(1) = &H01
            ndefRecordURI(2) = Convert.ToByte(ndefURIBytes.Length + 1)
            ndefRecordURI(3) = Text.Encoding.ASCII.GetBytes("U")(1)
            ndefRecordURI(4) = &H04

            ' Only short records are supported
            For i = 0 To ndefURIBytes.Length - 1
                ndefRecordURI(5 + i) = ndefURIBytes(i)
            Next

            sender.CardemuWriteNDEF(ndefRecordURI)
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
