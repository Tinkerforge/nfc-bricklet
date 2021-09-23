Imports System
Imports System.Threading
Imports Tinkerforge

Module ExampleSimple
    Const HOST As String = "localhost"
    Const PORT As Integer = 4223
    Const UID As String = "XYZ" ' Change XYZ to the UID of your NFC Bricklet

    Sub Main()
        Dim ipcon As New IPConnection() ' Create IP connection
        Dim nfc As New BrickletNFC(UID, ipcon) ' Create device object

        ipcon.Connect(HOST, PORT) ' Connect to brickd
        ' Don't use device before ipcon is connected

        ' Enable simple mode
        nfc.SetMode(BrickletNFC.MODE_SIMPLE)

        ' Get current tag IDs 10 times
        Dim cycle As Integer
        Dim index As Byte
        Dim tagType As Byte
        Dim tagID As Byte()
        Dim lastSeen As Long
        Dim tagInfo As String
        Dim i As Integer

        For cycle = 1 To 10
            Console.WriteLine("Cycle: " + cycle.ToString() + " of 10")

            For index = 0 To 7
                Try
                    nfc.SimpleGetTagID(index, tagType, tagID, lastSeen)
                Catch
                    ' On error enable simple mode again
                    nfc.SetMode(BrickletNFC.MODE_SIMPLE)
                    Continue For
                End Try

                If tagID.Length = 0 Then
                    Exit For
                End If

                tagInfo = "["

                For i = 0 To tagID.Length - 1
                    tagInfo += String.Format("0x{0:X2}", tagID(i))

                    If i < tagID.Length - 1 Then
                        tagInfo += " "
                    End If
                Next

                tagInfo += "]"

                Console.WriteLine("  Index: " + index.ToString() + _
                                  ", Tag Type: " + tagType.ToString() + _
                                  ", Tag ID: " + tagInfo + _
                                  ", Last Seen: " + (lastSeen / 1000.0).ToString() + " seconds ago")
            Next index

            Console.WriteLine("")
            Thread.Sleep(1000)
        Next cycle

        Console.WriteLine("Press key to exit")
        Console.ReadLine()
        ipcon.Disconnect()
    End Sub
End Module
