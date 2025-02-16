// Onat Bulut - 2024

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

#include <wayland-client.h>

#include "xdg-shell-client-protocol.h"

struct wl_buffer *buffer = NULL;
struct wl_compositor *compositor = NULL;
struct wl_shm *shm = NULL;
struct wl_surface *surface = NULL;

struct xdg_toplevel *xdg_toplevel = NULL;
struct xdg_wm_base *xdg_shell = NULL;

int width = 450;
int height = 300;
uint8_t *pixels = NULL;

bool cls = false;

int random_shared_memory_name(char *name, size_t size);

void redraw();

void surface_configure_handler(void *data, struct xdg_surface *xdg_surface, uint32_t serial)
{
    xdg_surface_ack_configure(xdg_surface, serial);

    if (!pixels)
        redraw();
}

struct xdg_surface_listener surface_listener = {
    .configure = surface_configure_handler
};


void toplevel_configure_handler(void *data, struct xdg_toplevel *top, int32_t new_width, int32_t new_height,
                                struct wl_array *states)
{
    if (new_width == 0 || new_height == 0)
        return;

    if (width != new_width || height != new_height)
    {
        munmap(pixels, width * height * 4);
        width = new_width;
        height = new_height;

        redraw();
    }
}

void toplevel_close_handler(void *data, struct xdg_toplevel *toplevel)
{
    cls = 1;
}

struct xdg_toplevel_listener toplevel_listener = {
    .configure = toplevel_configure_handler,
    .close = toplevel_close_handler
};

void shell_ping_handler(void *data, struct xdg_wm_base *shell, uint32_t serial)
{
    xdg_wm_base_pong(shell, serial);
}

struct xdg_wm_base_listener shell_listener = {
    .ping = shell_ping_handler
};

void registry_global_handler(void *data, struct wl_registry *registry, uint32_t name, const char *interface,
                             uint32_t version)
{
    if (strcmp(interface, wl_compositor_interface.name) == 0)
    {
        compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 4);
    }
    else if (strcmp(interface, wl_shm_interface.name) == 0)
    {
        shm = wl_registry_bind(registry, name, &wl_shm_interface, 1);
    }
    else if (strcmp(interface, xdg_wm_base_interface.name) == 0)
    {
        xdg_shell = wl_registry_bind(registry, name, &xdg_wm_base_interface, 3);
        xdg_wm_base_add_listener(xdg_shell, &shell_listener, NULL);
    }
}

void registry_global_remove_handler(void *data, struct wl_registry *registry, uint32_t name) {}

struct wl_registry_listener registry_listener = {
    .global = registry_global_handler,
    .global_remove = registry_global_remove_handler
};

int random_shared_memory_name(char *name, size_t size)
{
    FILE *urandom = fopen("/dev/urandom", "r");
    if (!urandom)
        return -1;

    name[0] = '/';
    if (fread(name + 1, 1, size - 2, urandom) != size - 2)
    {
        fclose(urandom);
        return -1;
    }
    fclose(urandom);

    for (int i = 1; i < size - 1; i++)
        name[i] = (char) ((name[i] & 23) + 'A');
    name[size - 1] = '\0';

    return 0;
}

void redraw()
{
    char name[8] = {0};
    random_shared_memory_name(name, 8);
    int fd = shm_open(name, O_RDWR | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    shm_unlink(name);
    ftruncate(fd, width * height * 4);

    pixels = mmap(NULL, width * height * 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    struct wl_shm_pool *pool = wl_shm_create_pool(shm, fd, width * height * 4);
    struct wl_buffer *new_buffer = wl_shm_pool_create_buffer(pool, 0, width, height, width * 4, WL_SHM_FORMAT_ARGB8888);

    if (buffer)
        wl_buffer_destroy(buffer);

    buffer = new_buffer;

    uint32_t *pixel_ptr = (uint32_t *) pixels;
    size_t total_pixels = width * height;

    for (size_t i = 0; i < total_pixels; i++)
    {
        pixel_ptr[i] = 0xCC000000;
    }

    wl_surface_attach(surface, buffer, 0, 0);
    wl_surface_damage_buffer(surface, 0, 0, width, height);
    wl_surface_commit(surface);

    wl_shm_pool_destroy(pool);
    close(fd);
}

int main(void)
{
    struct wl_display *display = wl_display_connect(NULL);
    struct wl_registry *registry = wl_display_get_registry(display);
    wl_registry_add_listener(registry, &registry_listener, NULL);
    wl_display_roundtrip(display);

    surface = wl_compositor_create_surface(compositor);

    struct xdg_surface *xdg_surface = xdg_wm_base_get_xdg_surface(xdg_shell, surface);
    xdg_surface_add_listener(xdg_surface, &surface_listener, NULL);
    xdg_toplevel = xdg_surface_get_toplevel(xdg_surface);
    xdg_toplevel_add_listener(xdg_toplevel, &toplevel_listener, NULL);
    xdg_toplevel_set_title(xdg_toplevel, "Wayland XDG Shell Example");
    xdg_toplevel_set_app_id(xdg_toplevel, "wl_xdg_shell_example");
    xdg_toplevel_set_min_size(xdg_toplevel, width, height);
    wl_surface_commit(surface);

    while (wl_display_dispatch(display))
        if (cls) break;

    if (buffer)
        wl_buffer_destroy(buffer);

    xdg_toplevel_destroy(xdg_toplevel);
    xdg_surface_destroy(xdg_surface);
    wl_surface_destroy(surface);
    wl_display_disconnect(display);

    return EXIT_SUCCESS;
}
