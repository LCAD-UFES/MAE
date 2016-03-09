find_path (XFORMS_INCLUDE_DIR forms.h
	PATHS /usr/include /usr/X11R6/include /usr/X11/include /usr/local/include)
find_library (XFORMS_LIBRARY 
	NAMES forms
	DOC "XForms library")
find_library (XFORMS_IMAGE_LIBRARY
	NAMES flimage
	DOC "XForms Image library")
if (XFORMS_INCLUDE_DIR AND XFORMS_LIBRARY AND XFORMS_IMAGE_LIBRARY)
	set (XFORMS_FOUND TRUE)
	set (XFORMS_LIBRARIES ${XFORMS_LIBRARY} ${XFORMS_IMAGE_LIBRARY})
endif ()

mark_as_advanced (XFORMS_INCLUDE_DIR XFORMS_LIBRARIES)

