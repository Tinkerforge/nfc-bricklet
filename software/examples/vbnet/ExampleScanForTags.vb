Imports System
Imports Tinkerforge

Module ExampleScanForTags
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your NFC Bricklet

    ' Callback subroutine for reader state changed callback
    Sub ReaderStateChangedCB(ByVal sender As BrickletNFC, ByVal state As Byte, _
                             ByVal idle As Boolean)
        If state = BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY Then
            Dim i As Integer
            Dim tagType As Byte
            Dim tagID() As Byte
            Dim tagInfo As String

            sender.ReaderGetTagID(tagType, tagID)

            tagInfo = String.Format("Found tag of type {0} with ID [", tagType)

            For i = 0 To tagID.Length - 1
                tagInfo += String.Format("0x{0:02X}", tagID(i))

                If i < tagID.Length - 1 Then
                    tagInfo += " "
                End If
            Next

            tagInfo += "]"

            Console.WriteLine(tagInfo)
        End If

        If idle Then
            sender.ReaderRequestTagID()
        End If
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim nfc As New BrickletNFC(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register reader state changed callback to subroutine ReaderStateChangedCB
        AddHandler nfc.ReaderStateChangedCallback, AddressOf ReaderStateChangedCB

        ' Enable reader mode
        nfc.SetMode(BrickletNFC.MODE_READER)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
