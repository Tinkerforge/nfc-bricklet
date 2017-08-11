# -*- coding: utf-8 -*-
#############################################################
# This file was automatically generated on 2017-08-11.      #
#                                                           #
# Python Bindings Version 2.1.14                            #
#                                                           #
# If you have a bugfix for this file and want to commit it, #
# please fix the bug in the generator. You can find a link  #
# to the generators git repository on tinkerforge.com       #
#############################################################

#### __DEVICE_IS_NOT_RELEASED__ ####

from collections import namedtuple

try:
    from .ip_connection import Device, IPConnection, Error, create_chunk_data
except ValueError:
    from ip_connection import Device, IPConnection, Error, create_chunk_data

ReaderGetTagIDLowLevel = namedtuple('ReaderGetTagIDLowLevel', ['tag_type', 'tag_id_length', 'tag_id_data'])
ReaderGetState = namedtuple('ReaderGetState', ['state', 'idle'])
ReaderReadNdefLowLevel = namedtuple('ReaderReadNdefLowLevel', ['ndef_length', 'ndef_chunk_offset', 'ndef_chunk_data'])
ReaderReadPageLowLevel = namedtuple('ReaderReadPageLowLevel', ['data_length', 'data_chunk_offset', 'data_chunk_data'])
CardemuGetState = namedtuple('CardemuGetState', ['state', 'idle'])
P2PGetState = namedtuple('P2PGetState', ['state', 'idle'])
P2PReadNdefLowLevel = namedtuple('P2PReadNdefLowLevel', ['ndef_length', 'ndef_chunk_offset', 'ndef_chunk_data'])
GetIdentity = namedtuple('Identity', ['uid', 'connected_uid', 'position', 'hardware_version', 'firmware_version', 'device_identifier'])
ReaderGetTagID = namedtuple('ReaderGetTagID', ['tag_type', 'tag_id'])

class BrickletNFC(Device):
    """
    TODO
    """

    DEVICE_IDENTIFIER = 286
    DEVICE_DISPLAY_NAME = 'NFC Bricklet'

    CALLBACK_READER_STATE_CHANGED = 13
    CALLBACK_CARDEMU_STATE_CHANGED = 18
    CALLBACK_P2P_STATE_CHANGED = 24


    FUNCTION_SET_MODE = 1
    FUNCTION_GET_MODE = 2
    FUNCTION_READER_REQUEST_TAG_ID = 3
    FUNCTION_READER_GET_TAG_ID_LOW_LEVEL = 4
    FUNCTION_READER_GET_STATE = 5
    FUNCTION_READER_WRITE_NDEF_LOW_LEVEL = 6
    FUNCTION_READER_REQUEST_NDEF = 7
    FUNCTION_READER_READ_NDEF_LOW_LEVEL = 8
    FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE = 9
    FUNCTION_READER_WRITE_PAGE_LOW_LEVEL = 10
    FUNCTION_READER_REQUEST_PAGE = 11
    FUNCTION_READER_READ_PAGE_LOW_LEVEL = 12
    FUNCTION_CARDEMU_GET_STATE = 14
    FUNCTION_CARDEMU_START_DISCOVERY = 15
    FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL = 16
    FUNCTION_CARDEMU_START_TRANSFER = 17
    FUNCTION_P2P_GET_STATE = 19
    FUNCTION_P2P_START_DISCOVERY = 20
    FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL = 21
    FUNCTION_P2P_START_TRANSFER = 22
    FUNCTION_P2P_READ_NDEF_LOW_LEVEL = 23
    FUNCTION_GET_IDENTITY = 255

    MODE_OFF = 0
    MODE_CARDEMU = 1
    MODE_P2P = 2
    MODE_READER = 3
    TAG_TYPE_MIFARE_CLASSIC = 0
    TAG_TYPE_TYPE1 = 1
    TAG_TYPE_TYPE2 = 2
    TAG_TYPE_TYPE3 = 3
    TAG_TYPE_TYPE4 = 4
    READER_STATE_INITIALIZATION = 0
    READER_STATE_IDLE = 128
    READER_STATE_ERROR = 192
    READER_STATE_REQUEST_TAG_ID = 2
    READER_STATE_REQUEST_TAG_ID_READY = 130
    READER_STATE_REQUEST_TAG_ID_ERROR = 194
    READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE = 3
    READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_READY = 131
    READER_STATE_AUTHENTICATE_MIFARE_CLASSIC_PAGE_ERROR = 195
    READER_STATE_WRITE_PAGE = 4
    READER_STATE_WRITE_PAGE_READY = 132
    READER_STATE_WRITE_PAGE_ERROR = 196
    READER_STATE_REQUEST_PAGE = 5
    READER_STATE_REQUEST_PAGE_READY = 133
    READER_STATE_REQUEST_PAGE_ERROR = 197
    READER_STATE_WRITE_NDEF = 6
    READER_STATE_WRITE_NDEF_READY = 134
    READER_STATE_WRITE_NDEF_ERROR = 198
    READER_STATE_REQUEST_NDEF = 7
    READER_STATE_REQUEST_NDEF_READY = 135
    READER_STATE_REQUEST_NDEF_ERROR = 199
    KEY_A = 0
    KEY_B = 1
    READER_WRITE_TYPE4_CAPABILITY_CONTAINER = 3
    READER_WRITE_TYPE4_NDEF = 4
    READER_REQUEST_TYPE4_CAPABILITY_CONTAINER = 3
    READER_REQUEST_TYPE4_NDEF = 4
    CARDEMU_STATE_INITIALIZATION = 0
    CARDEMU_STATE_IDLE = 128
    CARDEMU_STATE_ERROR = 192
    CARDEMU_STATE_DISCOVER = 2
    CARDEMU_STATE_DISCOVER_READY = 130
    CARDEMU_STATE_DISCOVER_ERROR = 194
    CARDEMU_STATE_TRANSFER_NDEF = 3
    CARDEMU_STATE_TRANSFER_NDEF_READY = 131
    CARDEMU_STATE_TRANSFER_NDEF_ERROR = 195
    CARDEMU_TRANSFER_ABORT = 0
    CARDEMU_TRANSFER_WRITE = 1
    P2P_STATE_INITIALIZATION = 0
    P2P_STATE_IDLE = 128
    P2P_STATE_ERROR = 192
    P2P_STATE_DISCOVER = 2
    P2P_STATE_DISCOVER_READY = 130
    P2P_STATE_DISCOVER_ERROR = 194
    P2P_STATE_TRANSFER_NDEF = 3
    P2P_STATE_TRANSFER_NDEF_READY = 131
    P2P_STATE_TRANSFER_NDEF_ERROR = 195
    P2P_TRANSFER_ABORT = 0
    P2P_TRANSFER_WRITE = 1
    P2P_TRANSFER_READ = 2

    def __init__(self, uid, ipcon):
        """
        Creates an object with the unique device ID *uid* and adds it to
        the IP Connection *ipcon*.
        """
        Device.__init__(self, uid, ipcon)

        self.api_version = (2, 0, 0)

        self.response_expected[BrickletNFC.FUNCTION_SET_MODE] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_GET_MODE] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_REQUEST_TAG_ID] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_READER_GET_TAG_ID_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_GET_STATE] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_WRITE_NDEF_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_REQUEST_NDEF] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_READER_READ_NDEF_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_READER_WRITE_PAGE_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletNFC.FUNCTION_READER_REQUEST_PAGE] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_READER_READ_PAGE_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_CARDEMU_GET_STATE] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_CARDEMU_START_DISCOVERY] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletNFC.FUNCTION_CARDEMU_START_TRANSFER] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_P2P_GET_STATE] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_P2P_START_DISCOVERY] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_TRUE
        self.response_expected[BrickletNFC.FUNCTION_P2P_START_TRANSFER] = BrickletNFC.RESPONSE_EXPECTED_FALSE
        self.response_expected[BrickletNFC.FUNCTION_P2P_READ_NDEF_LOW_LEVEL] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE
        self.response_expected[BrickletNFC.FUNCTION_GET_IDENTITY] = BrickletNFC.RESPONSE_EXPECTED_ALWAYS_TRUE

        self.callback_formats[BrickletNFC.CALLBACK_READER_STATE_CHANGED] = 'B !'
        self.callback_formats[BrickletNFC.CALLBACK_CARDEMU_STATE_CHANGED] = 'B !'
        self.callback_formats[BrickletNFC.CALLBACK_P2P_STATE_CHANGED] = 'B !'


    def set_mode(self, mode):
        """
        TODO
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_SET_MODE, (mode,), 'B', '')

    def get_mode(self):
        """
        TODO
        """
        return self.ipcon.send_request(self, BrickletNFC.FUNCTION_GET_MODE, (), '', 'B')

    def reader_request_tag_id(self):
        """
        TODO
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_REQUEST_TAG_ID, (), '', '')

    def reader_get_tag_id_low_level(self):
        """
        Returns the tag type, tag ID and the length of the tag ID
        (4 or 7 bytes are possible length). This function can only be called if the
        NFC Bricklet is currently in one of the *Ready* states. The returned ID
        is the ID that was saved through the last call of :func:`Reader Request Tag ID`.

        To get the tag ID of a tag the approach is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)
        3. Call :func:`Reader Get Tag ID`
        """
        return ReaderGetTagIDLowLevel(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_GET_TAG_ID_LOW_LEVEL, (), '', 'B B 32B'))

    def reader_get_state(self):
        """
        Returns the current state of the NFC Bricklet.

        On startup the Bricklet will be in the *Initialization* state. The
        initialization will only take about 20ms. After that it changes to *Idle*.

        The functions of this Bricklet can be called in the *Idle* state and all of
        the *Ready* and *Error* states.

        Example: If you call :func:`Reader Request Page`, the state will change to
        *RequestPage* until the reading of the page is finished. Then it will change
        to either *RequestPageReady* if it worked or to *RequestPageError* if it
        didn't. If the request worked you can get the page by calling :func:`Reader Read Page`.

        The same approach is used analogously for the other API functions.
        """
        return ReaderGetState(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_GET_STATE, (), '', 'B !'))

    def reader_write_ndef_low_level(self, ndef_length, ndef_chunk_offset, ndef_chunk_data):
        """
        works with type 2 and 4
        has to be ndef formated already
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_WRITE_NDEF_LOW_LEVEL, (ndef_length, ndef_chunk_offset, ndef_chunk_data), 'H H 60B', '')

    def reader_request_ndef(self):
        """

        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_REQUEST_NDEF, (), '', '')

    def reader_read_ndef_low_level(self):
        """
        TODO

        works with type 1-4
        """
        return ReaderReadNdefLowLevel(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_READ_NDEF_LOW_LEVEL, (), '', 'H H 60B'))

    def reader_authenticate_mifare_classic_page(self, page, key_number, key):
        """
        TODO:
        TODO: authenticating a page always authenticates a whole sector of 4 pages!
        TODO:


        Mifare Classic tags use authentication. If you want to read from or write to
        a Mifare Classic page you have to authenticate it beforehand.
        Each page can be authenticated with two keys: A (``key_number`` = 0) and B
        (``key_number`` = 1). A new Mifare Classic
        tag that has not yet been written to can can be accessed with key A
        and the default key ``[0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF]``.

        The approach to read or write a Mifare Classic page is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State`
           or :cb:`Reader State Changed` callback)
        3. If looking for a specific tag then call :func:`Reader Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Reader Authenticate Mifare Classic Page` with page and key for the page
        5. Wait for state to change to *AuthenticatingMifareClassicPageReady* (see
           :func:`Reader Get State` or :cb:`Reader State Changed` callback)
        6. Call :func:`Reader Request Page` or :func:`Reader Write Page` to read/write page
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_AUTHENTICATE_MIFARE_CLASSIC_PAGE, (page, key_number, key), 'H B 6B', '')

    def reader_write_page_low_level(self, page, data_length, data_chunk_offset, data_chunk_data):
        """
        Writes 16 bytes starting from the given page. How many pages are written
        depends on the tag type. The page sizes are as follows:

        * Mifare Classic page size: 16 byte (one page is written)
        * NFC Forum Type 1 page size: 8 byte (two pages are written)
        * NFC Forum Type 2 page size: 4 byte (four pages are written)
        * NFC Forum Type 3 page size: 16 byte (one page is written)

        * NFC Forum Type 4: no pages, page = file selection (cc or ndef)

        The general approach for writing to a tag is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)
        3. If looking for a specific tag then call :func:`Reader Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Reader Write Page` with page number and data
        5. Wait for state to change to *WritePageReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)

        If you use a Mifare Classic tag you have to authenticate a page before you
        can write to it. See :func:`Reader Authenticate Mifare Classic Page`.
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_WRITE_PAGE_LOW_LEVEL, (page, data_length, data_chunk_offset, data_chunk_data), 'H H H 58B', '')

    def reader_request_page(self, page, length):
        """
        Reads 16 bytes starting from the given page and stores them into a buffer.
        The buffer can then be read out with :func:`Reader Read Page`.
        How many pages are read depends on the tag type. The page sizes are
        as follows:

        * Mifare Classic page size: 16 byte (one page is read)
        * NFC Forum Type 1 page size: 8 byte (two pages are read)
        * NFC Forum Type 2 page size: 4 byte (four pages are read)

        The general approach for reading a tag is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State`
           or :cb:`Reader State Changed` callback)
        3. If looking for a specific tag then call :func:`Reader Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Reader Request Page` with page number
        5. Wait for state to change to *RequestPageReady* (see :func:`Reader Get State`
           or :cb:`Reader State Changed` callback)
        6. Call :func:`Reader Read Page` to retrieve the page from the buffer

        If you use a Mifare Classic tag you have to authenticate a page before you
        can read it. See :func:`Reader Authenticate Mifare Classic Page`.
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_REQUEST_PAGE, (page, length), 'H H', '')

    def reader_read_page_low_level(self):
        """
        TODO
        """
        return ReaderReadPageLowLevel(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_READER_READ_PAGE_LOW_LEVEL, (), '', 'H H 60B'))

    def cardemu_get_state(self):
        """
        TODO
        """
        return CardemuGetState(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_CARDEMU_GET_STATE, (), '', 'B !'))

    def cardemu_start_discovery(self):
        """
        TODO
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_CARDEMU_START_DISCOVERY, (), '', '')

    def cardemu_write_ndef_low_level(self, ndef_length, ndef_chunk_offset, ndef_chunk_data):
        """

        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_CARDEMU_WRITE_NDEF_LOW_LEVEL, (ndef_length, ndef_chunk_offset, ndef_chunk_data), 'H H 60B', '')

    def cardemu_start_transfer(self, transfer):
        """

        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_CARDEMU_START_TRANSFER, (transfer,), 'B', '')

    def p2p_get_state(self):
        """
        TODO
        """
        return P2PGetState(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_P2P_GET_STATE, (), '', 'B !'))

    def p2p_start_discovery(self):
        """
        TODO
        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_P2P_START_DISCOVERY, (), '', '')

    def p2p_write_ndef_low_level(self, ndef_length, ndef_chunk_offset, ndef_chunk_data):
        """

        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_P2P_WRITE_NDEF_LOW_LEVEL, (ndef_length, ndef_chunk_offset, ndef_chunk_data), 'H H 60B', '')

    def p2p_start_transfer(self, transfer):
        """

        """
        self.ipcon.send_request(self, BrickletNFC.FUNCTION_P2P_START_TRANSFER, (transfer,), 'B', '')

    def p2p_read_ndef_low_level(self):
        """
        TODO
        """
        return P2PReadNdefLowLevel(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_P2P_READ_NDEF_LOW_LEVEL, (), '', 'H H 60B'))

    def get_identity(self):
        """
        Returns the UID, the UID where the Bricklet is connected to,
        the position, the hardware and firmware version as well as the
        device identifier.

        The position can be 'a', 'b', 'c' or 'd'.

        The device identifier numbers can be found :ref:`here <device_identifier>`.
        |device_identifier_constant|
        """
        return GetIdentity(*self.ipcon.send_request(self, BrickletNFC.FUNCTION_GET_IDENTITY, (), '', '8s 8s c 3B 3B H'))

    def reader_get_tag_id(self):
        """
        Returns the tag type, tag ID and the length of the tag ID
        (4 or 7 bytes are possible length). This function can only be called if the
        NFC Bricklet is currently in one of the *Ready* states. The returned ID
        is the ID that was saved through the last call of :func:`Reader Request Tag ID`.

        To get the tag ID of a tag the approach is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)
        3. Call :func:`Reader Get Tag ID`
        """
        ret = self.reader_get_tag_id_low_level()

        return ReaderGetTagID(ret.tag_type, ret.tag_id_data[:ret.tag_id_length])

    def reader_write_ndef(self, ndef):
        """
        works with type 2 and 4
        has to be ndef formated already
        """
        if len(ndef) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Ndef can be at most 65535 items long')

        ndef = list(ndef) # convert potential tuple to list
        ndef_length = len(ndef)
        ndef_chunk_offset = 0

        if ndef_length == 0:
            ndef_chunk_data = [0] * 60
            ret = self.reader_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
        else:
            with self.stream_lock:
                while ndef_chunk_offset < ndef_length:
                    ndef_chunk_data = create_chunk_data(ndef, ndef_chunk_offset, 60, 0)
                    ret = self.reader_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
                    ndef_chunk_offset += 60

        return ret

    def reader_read_ndef(self):
        """
        TODO

        works with type 1-4
        """
        with self.stream_lock:
            ret = self.reader_read_ndef_low_level()
            ndef_length = ret.ndef_length
            ndef_out_of_sync = ret.ndef_chunk_offset != 0
            ndef_data = ret.ndef_chunk_data

            while not ndef_out_of_sync and len(ndef_data) < ndef_length:
                ret = self.reader_read_ndef_low_level()
                ndef_length = ret.ndef_length
                ndef_out_of_sync = ret.ndef_chunk_offset != len(ndef_data)
                ndef_data += ret.ndef_chunk_data

            if ndef_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.ndef_chunk_offset + 60 < ndef_length:
                    ret = self.reader_read_ndef_low_level()
                    ndef_length = ret.ndef_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Ndef stream is out-of-sync')

        return ndef_data[:ndef_length]

    def reader_write_page(self, page, data):
        """
        Writes 16 bytes starting from the given page. How many pages are written
        depends on the tag type. The page sizes are as follows:

        * Mifare Classic page size: 16 byte (one page is written)
        * NFC Forum Type 1 page size: 8 byte (two pages are written)
        * NFC Forum Type 2 page size: 4 byte (four pages are written)
        * NFC Forum Type 3 page size: 16 byte (one page is written)

        * NFC Forum Type 4: no pages, page = file selection (cc or ndef)

        The general approach for writing to a tag is as follows:

        1. Call :func:`Reader Request Tag ID`
        2. Wait for state to change to *RequestTagIDReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)
        3. If looking for a specific tag then call :func:`Reader Get Tag ID` and check if the
           expected tag was found, if it was not found got back to step 1
        4. Call :func:`Reader Write Page` with page number and data
        5. Wait for state to change to *WritePageReady* (see :func:`Reader Get State` or
           :cb:`Reader State Changed` callback)

        If you use a Mifare Classic tag you have to authenticate a page before you
        can write to it. See :func:`Reader Authenticate Mifare Classic Page`.
        """
        if len(data) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Data can be at most 65535 items long')

        data = list(data) # convert potential tuple to list
        data_length = len(data)
        data_chunk_offset = 0

        if data_length == 0:
            data_chunk_data = [0] * 58
            ret = self.reader_write_page_low_level(page, data_length, data_chunk_offset, data_chunk_data)
        else:
            with self.stream_lock:
                while data_chunk_offset < data_length:
                    data_chunk_data = create_chunk_data(data, data_chunk_offset, 58, 0)
                    ret = self.reader_write_page_low_level(page, data_length, data_chunk_offset, data_chunk_data)
                    data_chunk_offset += 58

        return ret

    def reader_read_page(self):
        """
        TODO
        """
        with self.stream_lock:
            ret = self.reader_read_page_low_level()
            data_length = ret.data_length
            data_out_of_sync = ret.data_chunk_offset != 0
            data_data = ret.data_chunk_data

            while not data_out_of_sync and len(data_data) < data_length:
                ret = self.reader_read_page_low_level()
                data_length = ret.data_length
                data_out_of_sync = ret.data_chunk_offset != len(data_data)
                data_data += ret.data_chunk_data

            if data_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.data_chunk_offset + 60 < data_length:
                    ret = self.reader_read_page_low_level()
                    data_length = ret.data_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Data stream is out-of-sync')

        return data_data[:data_length]

    def cardemu_write_ndef(self, ndef):
        """

        """
        if len(ndef) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Ndef can be at most 65535 items long')

        ndef = list(ndef) # convert potential tuple to list
        ndef_length = len(ndef)
        ndef_chunk_offset = 0

        if ndef_length == 0:
            ndef_chunk_data = [0] * 60
            ret = self.cardemu_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
        else:
            with self.stream_lock:
                while ndef_chunk_offset < ndef_length:
                    ndef_chunk_data = create_chunk_data(ndef, ndef_chunk_offset, 60, 0)
                    ret = self.cardemu_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
                    ndef_chunk_offset += 60

        return ret

    def p2p_write_ndef(self, ndef):
        """

        """
        if len(ndef) > 65535:
            raise Error(Error.INVALID_PARAMETER, 'Ndef can be at most 65535 items long')

        ndef = list(ndef) # convert potential tuple to list
        ndef_length = len(ndef)
        ndef_chunk_offset = 0

        if ndef_length == 0:
            ndef_chunk_data = [0] * 60
            ret = self.p2p_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
        else:
            with self.stream_lock:
                while ndef_chunk_offset < ndef_length:
                    ndef_chunk_data = create_chunk_data(ndef, ndef_chunk_offset, 60, 0)
                    ret = self.p2p_write_ndef_low_level(ndef_length, ndef_chunk_offset, ndef_chunk_data)
                    ndef_chunk_offset += 60

        return ret

    def p2p_read_ndef(self):
        """
        TODO
        """
        with self.stream_lock:
            ret = self.p2p_read_ndef_low_level()
            ndef_length = ret.ndef_length
            ndef_out_of_sync = ret.ndef_chunk_offset != 0
            ndef_data = ret.ndef_chunk_data

            while not ndef_out_of_sync and len(ndef_data) < ndef_length:
                ret = self.p2p_read_ndef_low_level()
                ndef_length = ret.ndef_length
                ndef_out_of_sync = ret.ndef_chunk_offset != len(ndef_data)
                ndef_data += ret.ndef_chunk_data

            if ndef_out_of_sync: # discard remaining stream to bring it back in-sync
                while ret.ndef_chunk_offset + 60 < ndef_length:
                    ret = self.p2p_read_ndef_low_level()
                    ndef_length = ret.ndef_length

                raise Error(Error.STREAM_OUT_OF_SYNC, 'Ndef stream is out-of-sync')

        return ndef_data[:ndef_length]

    def register_callback(self, callback_id, function):
        """
        Registers the given *function* with the given *callback_id*.
        """
        if function is None:
            self.registered_callbacks.pop(callback_id, None)
        else:
            self.registered_callbacks[callback_id] = function

NFC = BrickletNFC # for backward compatibility
