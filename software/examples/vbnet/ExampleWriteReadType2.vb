Imports System
Imports Tinkerforge

Module ExampleScanForTags
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your NFC Bricklet

    ' Callback subroutine for state changed callback
    Sub StateChangedCB(ByVal sender As BrickletNFC, _
                       ByVal state As Byte, _
                       ByVal idle As Boolean)
        If state = BrickletNFC.READER_STATE_IDLE Then
            sender.ReaderRequestTagID()
        ElseIf(state = BrickletNFC.READER_STATE_REQUEST_TAG_ID_READY) Then
            Dim tagType As Byte
            Dim tagID() As Byte

            sender.ReaderGetTagID(tagType, tagID)

            If tagType <> BrickletNFC.TAG_TYPE_TYPE2 Then
                Console.WriteLine("Tag is not type-2")

                return
            End If

            Console.WriteLine("Found tag of type {0} with ID [0x{1:X} 0x{2:X} 0x{3:X} 0x{4:X}]", _
                              tagType, _
                              tagID(0), _
                              tagID(1), _
                              tagID(2), _
                              tagID(3))
            sender.ReaderRequestPage(1, 4)
        ElseIf state = BrickletNFC.READER_STATE_REQUEST_TAG_ID_ERROR Then
            Console.WriteLine("Request tag ID error")
        ElseIf state = BrickletNFC.READER_STATE_REQUEST_PAGE_READY Then
            Dim page() As Byte

            page = sender.ReaderReadPage()

            Console.WriteLine("Page read: 0x{0:X} 0x{1:X} 0x{2:X} 0x{3:X}",
                              page(0),
                              page(1),
                              page(2),
                              page(3))
            sender.ReaderWritePage(1, page)
        ElseIf state = BrickletNFC.READER_STATE_WRITE_PAGE_READY Then
            Console.WriteLine("Write page ready")
        ElseIf state = BrickletNFC.READER_STATE_REQUEST_PAGE_ERROR Then
            Console.WriteLine("Request page error")
        ElseIf state = BrickletNFC.READER_STATE_WRITE_PAGE_ERROR Then
            Console.WriteLine("Write page error")
        End If
    End Sub

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim nfc As New BrickletNFC(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Register state changed callback to subroutine StateChangedCB
        AddHandler nfc.ReaderStateChangedCallback, AddressOf StateChangedCB

        nfc.SetMode(BrickletNFC.MODE_READER)

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
