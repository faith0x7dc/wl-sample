/*
 * The MIT License (MIT)
 *
 * Copyright © 2014 faith
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 *
 * refer: weston/simple-egl
 *   http://cgit.freedesktop.org/wayland/weston/
 */

/*======================================
	Header include
======================================*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <signal.h>

#include <wayland-client.h>
#include <wayland-egl.h>

#include <GLES2/gl2.h>
#include <EGL/egl.h>

#include "image-loader.h"


/*======================================
	Structures
======================================*/

struct window;
struct setting;

struct display {
	struct wl_display *display;
	struct wl_registry *registry;
	struct wl_compositor *compositor;
	struct wl_shell *shell;
	struct wl_seat *seat;
	struct wl_touch *touch;
	struct wl_pointer *pointer;
	struct wl_keyboard *keyboard;
	struct {
		EGLDisplay dpy;
		EGLContext ctx;
		EGLConfig conf;
	} egl;
	struct window *window;
	struct setting *setting;
};

struct geometry {
	int width, height;
};

struct window {
	struct display *display;
	struct geometry geometry, window_size;
	struct {
		GLuint pos;
		GLuint tex;
	} gl;

	uint32_t benchmark_time, frames;
	struct wl_egl_window *native;
	struct wl_surface *surface;
	struct wl_shell_surface *shell_surface;
	EGLSurface egl_surface;
	struct wl_callback *callback;
	int configured;

	struct {
		GLuint id;
		char *path;
		uint32_t *data;
		uint32_t width, height;
	} texture;
};

struct setting {
	char *title;
};

/*======================================
	Prototypes
======================================*/

static void usage(int error_code);

static void signal_int(int signum);

static void create_surface(struct window *window);
static void destroy_surface(struct window *window);

static void init_egl(struct display *display, struct window *window);
static void fini_egl(struct display *display);
static void init_gl(struct window *window);
static GLuint create_shader(struct window *window, const char *source, GLenum shader_type);

static void redraw(void *data, struct wl_callback *callback, uint32_t time);

static void load_texture(struct window *window);
static void draw_texture(struct window *window);

static void registry_handle_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
static void registry_handle_global_remove(void *data, struct wl_registry *registry, uint32_t name);

static void handle_ping(void *data, struct wl_shell_surface *shell_surface, uint32_t serial);
static void handle_configure(void *data, struct wl_shell_surface *shell_surface, uint32_t edges, int32_t width, int32_t height);
static void handle_popup_done(void *data, struct wl_shell_surface *shell_surface);

static void configure_callback(void *data, struct wl_callback *callback, uint32_t  time);

static void seat_handle_capabilities(void *data, struct wl_seat *seat, enum wl_seat_capability caps);

static void touch_handle_down(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, struct wl_surface *surface, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w);
static void touch_handle_up(void *data, struct wl_touch *wl_touch, uint32_t serial, uint32_t time, int32_t id);
static void touch_handle_motion(void *data, struct wl_touch *wl_touch, uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w);
static void touch_handle_frame(void *data, struct wl_touch *wl_touch);
static void touch_handle_cancel(void *data, struct wl_touch *wl_touch);

static void pointer_handle_enter(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface, wl_fixed_t sx, wl_fixed_t sy);
static void pointer_handle_leave(void *data, struct wl_pointer *pointer, uint32_t serial, struct wl_surface *surface);
static void pointer_handle_motion(void *data, struct wl_pointer *pointer, uint32_t time, wl_fixed_t sx, wl_fixed_t sy);
static void pointer_handle_button(void *data, struct wl_pointer *wl_pointer, uint32_t serial, uint32_t time, uint32_t button, uint32_t state);
static void pointer_handle_axis(void *data, struct wl_pointer *wl_pointer, uint32_t time, uint32_t axis, wl_fixed_t value);

static void keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard, uint32_t format, int fd, uint32_t size);
static void keyboard_handle_enter(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface, struct wl_array *keys);
static void keyboard_handle_leave(void *data, struct wl_keyboard *keyboard, uint32_t serial, struct wl_surface *surface);
static void keyboard_handle_key(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t time, uint32_t key, uint32_t state);
static void keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard, uint32_t serial, uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked, uint32_t group);


/*======================================
	Variables
======================================*/

static const char *vert_shader_text =
	"attribute vec4 pos;\n"
	"attribute vec2 tex;\n"
	"varying vec2 v_tex;\n"
	"void main() {\n"
	"  gl_Position = pos;\n"
	"  v_tex = tex;\n"
	"}\n";

static const char *frag_shader_text =
	"precision mediump float;\n"
	"varying vec2 v_tex;\n"
	"uniform sampler2D texture;\n"
	"void main() {\n"
	"  gl_FragColor = texture2D(texture, v_tex);\n"
	"}\n";

static const struct wl_registry_listener registry_listener = {
	registry_handle_global,
	registry_handle_global_remove
};

static const struct wl_shell_surface_listener shell_surface_listener = {
	handle_ping,
	handle_configure,
	handle_popup_done
};

static const struct wl_callback_listener configure_callback_listener = {
	configure_callback,
};

static const struct wl_callback_listener frame_listener = {
	redraw
};

static const struct wl_seat_listener seat_listener = {
	seat_handle_capabilities,
};

static const struct wl_pointer_listener pointer_listener = {
	pointer_handle_enter,
	pointer_handle_leave,
	pointer_handle_motion,
	pointer_handle_button,
	pointer_handle_axis,
};

static const struct wl_touch_listener touch_listener = {
	touch_handle_down,
	touch_handle_up,
	touch_handle_motion,
	touch_handle_frame,
	touch_handle_cancel,
};

static const struct wl_keyboard_listener keyboard_listener = {
	keyboard_handle_keymap,
	keyboard_handle_enter,
	keyboard_handle_leave,
	keyboard_handle_key,
	keyboard_handle_modifiers,
};

static int running = 1;


/*======================================
	Public functions
======================================*/

int
main(int argc, char **argv)
{
	struct sigaction sigint;
	struct display display = { 0 };
	struct window  window  = { 0 };
	struct setting setting = { 0 };
	int ret = 0, i;

	window.display = &display;
	display.window = &window;
	display.setting = &setting;
	window.window_size.width  = 250;
	window.window_size.height = 250;

	for (i = 1; i < argc; i++) {
		if ((strcmp("-t", argv[i]) == 0) && (i + 1 < argc) && !setting.title) {
			setting.title = strdup(argv[i + 1]);
			i++;
		} else if ((strcmp("-s", argv[i]) == 0) && (i + 1 < argc)) {
			sscanf(argv[i + 1], "%dx%d", &window.window_size.width, &window.window_size.height);
			i++;
		} else if (strcmp("-h", argv[i]) == 0) {
			usage(EXIT_SUCCESS);
		} else if (i + 1 == argc) {
			window.texture.path = argv[i];
		} else {
			usage(EXIT_FAILURE);
		}
	}

	if (!setting.title)
		setting.title = strdup("image-viewer-egl");

	display.display = wl_display_connect(NULL);
	assert(display.display);

	display.registry = wl_display_get_registry(display.display);
	wl_registry_add_listener(display.registry,
				 &registry_listener, &display);

	wl_display_dispatch(display.display);

	init_egl(&display, &window);
	create_surface(&window);
	init_gl(&window);

	sigint.sa_handler = signal_int;
	sigemptyset(&sigint.sa_mask);
	sigint.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &sigint, NULL);

	while (running && ret != -1) {
		wl_display_dispatch_pending(display.display);
		while (!window.configured)
			wl_display_dispatch(display.display);
		redraw(&window, NULL, 0);
	}

	fprintf(stderr, "square exiting\n");

	free(setting.title);

	destroy_surface(&window);
	fini_egl(&display);

	if (display.shell)
		wl_shell_destroy(display.shell);

	if (display.compositor)
		wl_compositor_destroy(display.compositor);

	wl_registry_destroy(display.registry);
	wl_display_flush(display.display);
	wl_display_disconnect(display.display);

	return 0;
}


/*======================================
	Inner functions
======================================*/

static void
usage(int error_code)
{
	fprintf(stderr, "Usage: image-viewer-egl [OPTIONS] <path to image file>\n\n"
		"  -t <title>\tSpecify title(default = image-viewer-egl)\n"
		"  -s <width>x<height>\tSpecify size(default = 250x250)\n"
		"  -h\tThis help text\n\n");

	exit(error_code);
}

static void
signal_int(int signum)
{
	running = 0;
}

static void
create_surface(struct window *window)
{
	struct display *display = window->display;
	EGLBoolean ret;

	window->surface = wl_compositor_create_surface(display->compositor);
	window->shell_surface = wl_shell_get_shell_surface(display->shell,
							   window->surface);

	wl_shell_surface_add_listener(window->shell_surface,
				      &shell_surface_listener, window);

	window->native =
		wl_egl_window_create(window->surface,
				     window->window_size.width,
				     window->window_size.height);
	window->egl_surface =
		eglCreateWindowSurface(display->egl.dpy,
				       display->egl.conf,
				       window->native, NULL);

	wl_shell_surface_set_title(window->shell_surface, display->setting->title);

	ret = eglMakeCurrent(window->display->egl.dpy, window->egl_surface,
			     window->egl_surface, window->display->egl.ctx);
	assert(ret == EGL_TRUE);

	wl_shell_surface_set_toplevel(window->shell_surface);
	handle_configure(window, window->shell_surface, 0,
		window->window_size.width,
		window->window_size.height);
	window->configured = 1;
}

static void
destroy_surface(struct window *window)
{
	eglMakeCurrent(window->display->egl.dpy, EGL_NO_SURFACE, EGL_NO_SURFACE,
		       EGL_NO_CONTEXT);

	eglDestroySurface(window->display->egl.dpy, window->egl_surface);
	wl_egl_window_destroy(window->native);

	wl_shell_surface_destroy(window->shell_surface);
	wl_surface_destroy(window->surface);

	if (window->callback)
		wl_callback_destroy(window->callback);
}

static void
init_egl(struct display *display, struct window *window)
{
	static const EGLint context_attribs[] = {
		EGL_CONTEXT_CLIENT_VERSION, 2,
		EGL_NONE
	};

	EGLint config_attribs[] = {
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RED_SIZE, 1,
		EGL_GREEN_SIZE, 1,
		EGL_BLUE_SIZE, 1,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NONE
	};

	EGLint major, minor, n, count, i, size;
	EGLConfig *configs;
	EGLBoolean ret;

	display->egl.dpy = eglGetDisplay(display->display);
	assert(display->egl.dpy);

	ret = eglInitialize(display->egl.dpy, &major, &minor);
	assert(ret == EGL_TRUE);
	ret = eglBindAPI(EGL_OPENGL_ES_API);
	assert(ret == EGL_TRUE);

	if (!eglGetConfigs(display->egl.dpy, NULL, 0, &count) || count < 1)
		assert(0);

	configs = calloc(count, sizeof *configs);
	assert(configs);

	ret = eglChooseConfig(display->egl.dpy, config_attribs,
			      configs, count, &n);
	assert(ret && n >= 1);

	for (i = 0; i < n; i++) {
		eglGetConfigAttrib(display->egl.dpy,
				   configs[i], EGL_BUFFER_SIZE, &size);
		if (size == 32) {
			display->egl.conf = configs[i];
			break;
		}
	}
	free(configs);
	if (display->egl.conf == NULL) {
		fprintf(stderr, "did not find config with buffer size 32\n");
		exit(EXIT_FAILURE);
	}

	display->egl.ctx = eglCreateContext(display->egl.dpy,
					    display->egl.conf,
					    EGL_NO_CONTEXT, context_attribs);
	assert(display->egl.ctx);
}

static void
fini_egl(struct display *display)
{
	eglTerminate(display->egl.dpy);
	eglReleaseThread();
}

static void
init_gl(struct window *window)
{
	GLuint frag, vert;
	GLuint program;
	GLint status;

	frag = create_shader(window, frag_shader_text, GL_FRAGMENT_SHADER);
	vert = create_shader(window, vert_shader_text, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		fprintf(stderr, "Error: linking:\n%*s\n", len, log);
		exit(1);
	}

	glUseProgram(program);
	
	window->gl.pos = 0;
	window->gl.tex = 1;

	glBindAttribLocation(program, window->gl.pos, "pos");
	glBindAttribLocation(program, window->gl.tex, "tex");
	glLinkProgram(program);

	window->texture.id = glGetUniformLocation(program, "texture");

	load_texture(window);
}

static GLuint
create_shader(struct window *window, const char *source, GLenum shader_type)
{
	GLuint shader;
	GLint status;

	shader = glCreateShader(shader_type);
	assert(shader != 0);

	glShaderSource(shader, 1, (const char **) &source, NULL);
	glCompileShader(shader);

	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (!status) {
		char log[1000];
		GLsizei len;
		glGetShaderInfoLog(shader, 1000, &len, log);
		fprintf(stderr, "Error: compiling %s: %*s\n",
			shader_type == GL_VERTEX_SHADER ? "vertex" : "fragment",
			len, log);
		exit(1);
	}

	return shader;
}

static void
redraw(void *data, struct wl_callback *callback, uint32_t time)
{
	struct window *window = data;
	struct display *display = window->display;

	assert(window->callback == callback);
	window->callback = NULL;

	if (callback)
		wl_callback_destroy(callback);

	if (!window->configured)
		return;

	glViewport(0, 0, window->geometry.width, window->geometry.height);

	glClearColor(0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT);

	draw_texture(window);

	eglSwapBuffers(display->egl.dpy, window->egl_surface);
}

static void
load_texture(struct window *window)
{
	struct image *image;
	uint32_t i;

	if (!window)
		return;

	if (!window->texture.path)
		return;

	image = load_png(window->texture.path);
	if (!image)
		return;

	window->texture.width = image->width;
	window->texture.height = image->height;

	/*
	 * convert BGRA -> RGBA
	 *
	 * Microsoft DIB: BGRA(ARGB in Big endian)
	 * OpenGL: RGBA
	 */
	for (i = 0; i < image->width * image->height; i++) {
		uint32_t tmp = image->data[i];

		image->data[i] = ( tmp        & 0xff00ff00) |
						 ((tmp >> 16) & 0x000000ff) |
						 ((tmp << 16) & 0x00ff0000);
	}

	glGenTextures(1, &window->texture.id);
	glBindTexture(GL_TEXTURE_2D, window->texture.id);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height,
		0, GL_RGBA, GL_UNSIGNED_BYTE, image->data);

	glBindTexture(GL_TEXTURE_2D, 0);

	free(image->data);
	free(image);
}

static void
draw_texture(struct window *window)
{
	GLfloat verts[4][2] = {
		{ -1,  1 },
		{ -1, -1 },
		{  1,  1 },
		{  1, -1 },
	};

	float tex_w = (window->geometry.width >= window->texture.width) ?
		1.0f : (float)window->geometry.width / window->texture.width;

	float tex_h = (window->geometry.height >= window->texture.height) ?
		1.0f : (float)window->geometry.height / window->texture.height;

	GLfloat tex[4][2] = {
		{     0,     0 },
		{     0, tex_h },
		{ tex_w,     0 },
		{ tex_w, tex_h },
	};

	glBindTexture(GL_TEXTURE_2D, window->texture.id);

	glVertexAttribPointer(window->gl.pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
	glVertexAttribPointer(window->gl.tex, 2, GL_FLOAT, GL_FALSE, 0, tex);

	glEnableVertexAttribArray(window->gl.pos);
	glEnableVertexAttribArray(window->gl.tex);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glDisableVertexAttribArray(window->gl.pos);
	glDisableVertexAttribArray(window->gl.tex);

	glBindTexture(GL_TEXTURE_2D, 0);
}

static void
registry_handle_global(void *data, struct wl_registry *registry,
	uint32_t name, const char *interface, uint32_t version)
{
	struct display *d = data;

	if (strcmp(interface, "wl_compositor") == 0) {
		d->compositor =
			wl_registry_bind(registry, name,
					 &wl_compositor_interface, 1);
	} else if (strcmp(interface, "wl_shell") == 0) {
		d->shell = wl_registry_bind(registry, name,
					    &wl_shell_interface, 1);
	} else if (strcmp(interface, "wl_seat") == 0) {
		d->seat = wl_registry_bind(registry, name,
					   &wl_seat_interface, 1);
		wl_seat_add_listener(d->seat, &seat_listener, d);
	}
}

static void
registry_handle_global_remove(void *data, struct wl_registry *registry,
	uint32_t name)
{
}

static void
handle_ping(void *data, struct wl_shell_surface *shell_surface,
	uint32_t serial)
{
	wl_shell_surface_pong(shell_surface, serial);
}

static void
handle_configure(void *data, struct wl_shell_surface *shell_surface,
	uint32_t edges, int32_t width, int32_t height)
{
	struct window *window = data;

	if (window->native)
		wl_egl_window_resize(window->native, width, height, 0, 0);

	window->geometry.width = width;
	window->geometry.height = height;

	window->window_size = window->geometry;
}

static void
handle_popup_done(void *data, struct wl_shell_surface *shell_surface)
{
}

static void
configure_callback(void *data, struct wl_callback *callback, uint32_t  time)
{
	struct window *window = data;

	wl_callback_destroy(callback);

	window->configured = 1;
}

static void
seat_handle_capabilities(void *data, struct wl_seat *seat,
	enum wl_seat_capability caps)
{
	struct display *d = data;

	if ((caps & WL_SEAT_CAPABILITY_TOUCH) && !d->touch) {
		d->touch = wl_seat_get_touch(seat);
		wl_touch_set_user_data(d->touch, d);
		wl_touch_add_listener(d->touch, &touch_listener, d);
	} else if (!(caps & WL_SEAT_CAPABILITY_TOUCH) && d->touch) {
		wl_touch_destroy(d->touch);
		d->touch = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_POINTER) && !d->pointer) {
		d->pointer = wl_seat_get_pointer(seat);
		wl_pointer_add_listener(d->pointer, &pointer_listener, d);
	} else if (!(caps & WL_SEAT_CAPABILITY_POINTER) && d->pointer) {
		wl_pointer_destroy(d->pointer);
		d->pointer = NULL;
	}

	if ((caps & WL_SEAT_CAPABILITY_KEYBOARD) && !d->keyboard) {
		d->keyboard = wl_seat_get_keyboard(seat);
		wl_keyboard_add_listener(d->keyboard, &keyboard_listener, d);
	} else if (!(caps & WL_SEAT_CAPABILITY_KEYBOARD) && d->keyboard) {
		wl_keyboard_destroy(d->keyboard);
		d->keyboard = NULL;
	}
}

static void
touch_handle_down(void *data, struct wl_touch *wl_touch,
	uint32_t serial, uint32_t time, struct wl_surface *surface,
	int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
}

static void
touch_handle_up(void *data, struct wl_touch *wl_touch,
	uint32_t serial, uint32_t time, int32_t id)
{
}

static void
touch_handle_motion(void *data, struct wl_touch *wl_touch,
	uint32_t time, int32_t id, wl_fixed_t x_w, wl_fixed_t y_w)
{
}

static void
touch_handle_frame(void *data, struct wl_touch *wl_touch)
{
}

static void
touch_handle_cancel(void *data, struct wl_touch *wl_touch)
{
}

static void
pointer_handle_enter(void *data, struct wl_pointer *pointer,
	uint32_t serial, struct wl_surface *surface,
	wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
pointer_handle_leave(void *data, struct wl_pointer *pointer,
	uint32_t serial, struct wl_surface *surface)
{
}

static void
pointer_handle_motion(void *data, struct wl_pointer *pointer,
	uint32_t time, wl_fixed_t sx, wl_fixed_t sy)
{
}

static void
pointer_handle_button(void *data, struct wl_pointer *wl_pointer,
	uint32_t serial, uint32_t time, uint32_t button,
	uint32_t state)
{
}

static void
pointer_handle_axis(void *data, struct wl_pointer *wl_pointer,
	uint32_t time, uint32_t axis, wl_fixed_t value)
{
}

static void
keyboard_handle_keymap(void *data, struct wl_keyboard *keyboard,
	uint32_t format, int fd, uint32_t size)
{
}

static void
keyboard_handle_enter(void *data, struct wl_keyboard *keyboard,
	uint32_t serial, struct wl_surface *surface,
	struct wl_array *keys)
{
}

static void
keyboard_handle_leave(void *data, struct wl_keyboard *keyboard,
	uint32_t serial, struct wl_surface *surface)
{
}

static void
keyboard_handle_key(void *data, struct wl_keyboard *keyboard,
	uint32_t serial, uint32_t time, uint32_t key,
	uint32_t state)
{
}

static void
keyboard_handle_modifiers(void *data, struct wl_keyboard *keyboard,
	uint32_t serial, uint32_t mods_depressed,
	uint32_t mods_latched, uint32_t mods_locked,
	uint32_t group)
{
}

