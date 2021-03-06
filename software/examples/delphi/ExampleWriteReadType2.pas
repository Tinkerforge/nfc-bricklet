program ExampleWriteReadType2;

{$ifdef MSWINDOWS}{$apptype CONSOLE}{$endif}
{$ifdef FPC}{$mode OBJFPC}{$H+}{$endif}

uses
  SysUtils, IPConnection, BrickletNFC;

type
  TExample = class
  private
    ipcon: TIPConnection;
    nfc: TBrickletNFC;
  public
    procedure ReaderStateChangedCB(sender: TBrickletNFC; const state: byte;
                                   const idle: boolean);
    procedure Execute;
  end;

const
  HOST = 'localhost';
  PORT = 4223;
  UID = 'XYZ'; { Change XYZ to the UID of your NFC Bricklet }

var
  e: TExample;

{ Callback procedure for reader state changed callback }
procedure TExample.ReaderStateChangedCB(sender: TBrickletNFC; const state: byte;
                                        const idle: boolean);
  var tagType: byte;
  var page: TArrayOfUInt8;
  var tagID: TArrayOfUInt8;
begin
  if state = BRICKLET_NFC_READER_STATE_IDLE then begin
    sender.ReaderRequestTagID;
  end
  else if state = BRICKLET_NFC_READER_STATE_REQUEST_TAG_ID_READY then begin
    sender.ReaderGetTagID(tagType, tagID);

    if (tagType <> BRICKLET_NFC_TAG_TYPE_TYPE2) then begin
      WriteLn('Tag is not type-2');
      exit;
    end;

    WriteLn('Found tag of type ' + IntToStr(tagType) + ' with ID [' +
            Format('0x%X', [tagID[0]]) + ' ' +
            Format('0x%X', [tagID[1]]) + ' ' +
            Format('0x%X', [tagID[2]]) + ' ' +
            Format('0x%X', [tagID[3]]) + ']');
    sender.readerRequestPage(1, 4);
  end
  else if state = BRICKLET_NFC_READER_STATE_REQUEST_TAG_ID_ERROR then begin
    WriteLn('Request tag ID error');
  end
  else if state = BRICKLET_NFC_READER_STATE_REQUEST_PAGE_READY then begin
    page := sender.ReaderReadPage;
    WriteLn('Read page: ' +
            Format('0x%X', [page[0]]) + ' ' +
            Format('0x%X', [page[1]]) + ' ' +
            Format('0x%X', [page[2]]) + ' ' +
            Format('0x%X', [page[3]]));
    sender.ReaderWritePage(1, page);
  end
  else if state = BRICKLET_NFC_READER_STATE_WRITE_PAGE_READY then begin
    WriteLn('Write page ready');
  end
  else if state = BRICKLET_NFC_READER_STATE_REQUEST_PAGE_ERROR then begin
    WriteLn('Request page error');
  end
  else if state = BRICKLET_NFC_READER_STATE_WRITE_PAGE_ERROR then begin
    WriteLn('Write page error');
  end;
end;

procedure TExample.Execute;
begin
  { Create IP connection }
  ipcon := TIPConnection.Create;

  { Create device object }
  nfc := TBrickletNFC.Create(UID, ipcon);

  { Connect to brickd }
  ipcon.Connect(HOST, PORT);
  { Don't use device before ipcon is connected }

  { Register reader state changed callback to procedure ReaderStateChangedCB }
  nfc.OnReaderStateChanged := {$ifdef FPC}@{$endif}ReaderStateChangedCB;

  { Enable reader mode }
  nfc.SetMode(BRICKLET_NFC_MODE_READER);

  WriteLn('Press key to exit');
  ReadLn;
  ipcon.Destroy; { Calls ipcon.Disconnect internally }
end;

begin
  e := TExample.Create;
  e.Execute;
  e.Destroy;
end.
