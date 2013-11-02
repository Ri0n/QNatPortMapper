
QT *= core network

SOURCES += $$PWD/natportmapper.cpp

HEADERS  += $$PWD/natportmapper.h

win32 {
	qaxcontainer {
		# to generate type library use something like this
		#C:\Program Files\Microsoft Visual Studio 9.0\vc\bin>"C:\Program Files\Microsoft SDKs\Windows\v6.0A\bin\midl.exe" /out F:\projects /newtlb "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include\natupnp.idl"
		TYPELIBS = $$PWD/natupnp.tlb
		SOURCES += $$PWD/natportmapper_win_activeqt.cpp
		HEADERS += $$PWD/natportmapper_win_activeqt.h
		DEFINES += NPM_ACTIVEQT
	} else {
		SOURCES += $$PWD/natportmapper_win.cpp
		HEADERS  += $$PWD/natportmapper_win.h
		LIBS += -lole32 -loleaut32
	}
}

INCLUDEPATH += $$PWD
