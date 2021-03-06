#pragma once

namespace Kore {
	enum KeyCode {
		KeyUnknown = 0,
		KeyBack = 1, // Android
		KeyCancel = 3,
		KeyHelp = 6,
		KeyBackspace = 8,
		KeyTab = 9,
		KeyClear = 12,
		KeyReturn = 13,
		KeyShift = 16,
		KeyControl = 17,
		KeyAlt = 18,
		KeyPause = 19,
		KeyCapsLock = 20,
		KeyKana = 21,
		KeyHangul = 21,
		KeyEisu = 22,
		KeyJunja = 23,
		KeyFinal = 24,
		KeyHanja = 25,
		KeyKanji = 25,
		KeyEscape = 27,
		KeyConvert = 28,
		KeyNonConvert = 29,
		KeyAccept = 30,
		KeyModeChange = 31,
		KeySpace = 32,
		KeyPageUp = 33,
		KeyPageDown = 34,
		KeyEnd = 35,
		KeyHome = 36,
		KeyLeft = 37,
		KeyUp = 38,
		KeyRight = 39,
		KeyDown = 40,
		KeySelect = 41,
		KeyPrint = 42,
		KeyExecute = 43,
		KeyPrintScreen = 44,
		KeyInsert = 45,
		KeyDelete = 46,
		Key0 = 48,
		Key1 = 49,
		Key2 = 50,
		Key3 = 51,
		Key4 = 52,
		Key5 = 53,
		Key6 = 54,
		Key7 = 55,
		Key8 = 56,
		Key9 = 57,
		KeyColon = 58,
		KeySemicolon = 59,
		KeyLessThan = 60,
		KeyEquals = 61,
		KeyGreaterThan = 62,
		KeyQuestionMark = 63,
		KeyAt = 64,
		KeyA = 65,
		KeyB = 66,
		KeyC = 67,
		KeyD = 68,
		KeyE = 69,
		KeyF = 70,
		KeyG = 71,
		KeyH = 72,
		KeyI = 73,
		KeyJ = 74,
		KeyK = 75,
		KeyL = 76,
		KeyM = 77,
		KeyN = 78,
		KeyO = 79,
		KeyP = 80,
		KeyQ = 81,
		KeyR = 82,
		KeyS = 83,
		KeyT = 84,
		KeyU = 85,
		KeyV = 86,
		KeyW = 87,
		KeyX = 88,
		KeyY = 89,
		KeyZ = 90,
		KeyWin = 91,
		KeyContextMenu = 93,
		KeySleep = 95,
		KeyNumpad0 = 96,
		KeyNumpad1 = 97,
		KeyNumpad2 = 98,
		KeyNumpad3 = 99,
		KeyNumpad4 = 100,
		KeyNumpad5 = 101,
		KeyNumpad6 = 102,
		KeyNumpad7 = 103,
		KeyNumpad8 = 104,
		KeyNumpad9 = 105,
		KeyMultiply = 106,
		KeyAdd = 107,
		KeySeparator = 108,
		KeySubtract = 109,
		KeyDecimal = 110,
		KeyDivide = 111,
		KeyF1 = 112,
		KeyF2 = 113,
		KeyF3 = 114,
		KeyF4 = 115,
		KeyF5 = 116,
		KeyF6 = 117,
		KeyF7 = 118,
		KeyF8 = 119,
		KeyF9 = 120,
		KeyF10 = 121,
		KeyF11 = 122,
		KeyF12 = 123,
		KeyF13 = 124,
		KeyF14 = 125,
		KeyF15 = 126,
		KeyF16 = 127,
		KeyF17 = 128,
		KeyF18 = 129,
		KeyF19 = 130,
		KeyF20 = 131,
		KeyF21 = 132,
		KeyF22 = 133,
		KeyF23 = 134,
		KeyF24 = 135,
		KeyNumLock = 144,
		KeyScrollLock = 145,
		KeyWinOemFjJisho = 146,
		KeyWinOemFjMasshou = 147,
		KeyWinOemFjTouroku = 148,
		KeyWinOemFjLoya = 149,
		KeyWinOemFjRoya = 150,
		KeyCircumflex = 160,
		KeyExclamation = 161,
		KeyDoubleQuote = 162,
		KeyHash = 163,
		KeyDollar = 164,
		KeyPercent = 165,
		KeyAmpersand = 166,
		KeyUnderscore = 167,
		KeyOpenParen = 168,
		KeyCloseParen = 169,
		KeyAsterisk = 170,
		KeyPlus = 171,
		KeyPipe = 172,
		KeyHyphenMinus = 173,
		KeyOpenCurlyBracket = 174,
		KeyCloseCurlyBracket = 175,
		KeyTilde = 176,
		KeyVolumeMute = 181,
		KeyVolumeDown = 182,
		KeyVolumeUp = 183,
		KeyComma = 188,
		KeyPeriod = 190,
		KeySlash = 191,
		KeyBackQuote = 192,
		KeyOpenBracket = 219,
		KeyBackSlash = 220,
		KeyCloseBracket = 221,
		KeyQuote = 222,
		KeyMeta = 224,
		KeyAltGr = 225,
		KeyWinIcoHelp = 227,
		KeyWinIco00 = 228,
		KeyWinIcoClear = 230,
		KeyWinOemReset = 233,
		KeyWinOemJump = 234,
		KeyWinOemPA1 = 235,
		KeyWinOemPA2 = 236,
		KeyWinOemPA3 = 237,
		KeyWinOemWSCTRL = 238,
		KeyWinOemCUSEL = 239,
		KeyWinOemATTN = 240,
		KeyWinOemFinish = 241,
		KeyWinOemCopy = 242,
		KeyWinOemAuto = 243,
		KeyWinOemENLW = 244,
		KeyWinOemBackTab = 245,
		KeyATTN = 246,
		KeyCRSEL = 247,
		KeyEXSEL = 248,
		KeyEREOF = 249,
		KeyPlay = 250,
		KeyZoom = 251,
		KeyPA1 = 253,
		KeyWinOemClear = 254
	};

	class Keyboard {
	public:
		static Keyboard* the();
		void (*KeyDown)(KeyCode code);
		void (*KeyUp)(KeyCode code);
		void (*KeyPress)(wchar_t character);
		void clear();

		// called by backend
		void _keydown(KeyCode code);
		void _keyup(KeyCode code);
		void _keypress(wchar_t character);
	};
}
