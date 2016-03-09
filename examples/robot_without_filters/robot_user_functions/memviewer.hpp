#ifndef _MEM_VIEWER_H
#define _MEM_VIEWER_H

// Definitions
#define MEM_VIEWER_VERSION              "1.0"
#define MEM_VIEWER_WINDOW_TITLE		"Memory Viewer"
#define MEM_VIEWER_DEFAULT_FOVY		50.0f
#define MEM_VIEWER_DEFAULT_ASPECT_RATIO	1.0f
#define MEM_VIEWER_DEFAULT_NEAR		.1f
#define MEM_VIEWER_DEFAULT_FAR		100000.0f
#define MEM_VIEWER_DEFAULT_FOCUS	400.0f

#define MEM_VIEWER_NEURON_MEMORY_SIZE	0
#define MEM_VIEWER_MEMORY_PATTERN_SIZE	1

// Macros

// Types

// Exported Variables

// Prototypes
#ifdef __cplusplus
extern "C" {
#endif

// Must be called before the module initialization
GLint MemViewerSetParameteri (GLint nParameter, GLint nValue);
GLint MemViewerInitialize ();
GLint MemViewerUpdateMemory (GLubyte *pattern, GLfloat associated_value);
GLint MemViewerClearMemory();
GLint MemViewerRefresh ();
GLint MemViewerQuit ();

#ifdef __cplusplus
}
#endif

#endif
