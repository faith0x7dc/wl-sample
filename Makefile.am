bin_PROGRAMS = \
	solid-surf-egl \
	square \
	double-square \
	square-shm \
	event-test

solid_surf_egl_SOURCES = solid-surf-egl.c
solid_surf_egl_CPPFLAGS = $(WAYLAND_CLIENT_CFLAGS) $(WAYLAND_CURSOR_CFLAGS) $(WAYLAND_EGL_CFLAGS) $(EGL_CFLAGS)
solid_surf_egl_LDADD = $(WAYLAND_CLIENT_LIBS) $(WAYLAND_CURSOR_LIBS) $(WAYLAND_EGL_LIBS) $(EGL_LIBS) -lGLESv2 -lm

square_SOURCES = square.c
square_CPPFLAGS = $(WAYLAND_CLIENT_CFLAGS) $(WAYLAND_EGL_CFLAGS) $(EGL_CFLAGS)
square_LDADD = $(WAYLAND_CLIENT_LIBS) $(WAYLAND_EGL_LIBS) $(EGL_LIBS) -lGLESv2

double_square_SOURCES = double-square.c
double_square_CPPFLAGS = $(WAYLAND_CLIENT_CFLAGS) $(WAYLAND_EGL_CFLAGS) $(EGL_CFLAGS)
double_square_LDADD = $(WAYLAND_CLIENT_LIBS) $(WAYLAND_EGL_LIBS) $(EGL_LIBS) -lGLESv2 -lpthread

square_shm_SOURCES = square-shm.c
square_shm_CPPFLAGS = $(WAYLAND_CLIENT_CFLAGS)
square_shm_LDADD = $(WAYLAND_CLIENT_LIBS)

event_test_SOURCES = event-test.c event.c
event_test_CPPFLAGS = $(WAYLAND_CLIENT_CFLAGS)
event_test_LDADD = $(WAYLAND_CLIENT_LIBS) -lpthread

