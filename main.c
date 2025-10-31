#include <screen/screen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void list_available_displays(screen_context_t screen_ctx) {
    screen_display_t *displays = NULL;
    int num_displays = 0;

    // Get all available displays
    int err = screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &num_displays);
    if (err == 0 && num_displays > 0) {
        displays = malloc(num_displays * sizeof(screen_display_t));
        screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void**)displays);

        printf("Available displays:\n");
        for (int i = 0; i < num_displays; i++) {
            int display_id, size[2];
            screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_ID, &display_id);
            screen_get_display_property_iv(displays[i], SCREEN_PROPERTY_SIZE, size);
            printf("  Display %d: %dx%d\n", display_id, size[0], size[1]);
        }
        free(displays);
    } else {
        printf("No displays found or error querying displays\n");
    }
}

int main(int argc, char **argv) {
    screen_context_t screen_ctx;
    screen_window_t window1, window2;
    int rect1[4] = {100, 100, 500, 500};  // x, y, width, height
    int rect2[4] = {0, 0, 1000, 1000};
    int usage = SCREEN_USAGE_NATIVE;
    int zorder1 = -1, zorder2 = 0;  // window1 behind window2
    int transparency = 128;  // 50% transparency (0-255)
    int sensitivity_on = 1;  // Sensitive to touch
    int sensitivity_off = 0; // Not sensitive to touch
    int err;

    // Create screen context
    err = screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
    if (err) {
        perror("screen_create_context");
        return EXIT_FAILURE;
    }
// List available displays
    list_available_displays(screen_ctx);

    // Create first window (background window)
    err = screen_create_window(&window1, screen_ctx);
    if (err) {
        perror("screen_create_window1");
        return EXIT_FAILURE;
    }

    // Create second window (transparent overlay)
    err = screen_create_window(&window2, screen_ctx);
    if (err) {
        perror("screen_create_window2");
        return EXIT_FAILURE;
    }
    int d =0;
    int *display_id = &d;
    // Parse command line argument for display ID if provided
    if (argc > 1) {
        (*display_id) = atoi(argv[1]);
    }

    // Set window usage
    screen_set_window_property_iv(window1, SCREEN_PROPERTY_USAGE, &usage);
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_USAGE, &usage);

    // Set window sizes
    screen_set_window_property_iv(window1, SCREEN_PROPERTY_BUFFER_SIZE, rect1 + 2);
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_BUFFER_SIZE, rect2 + 2);

    // Set window positions
    screen_set_window_property_iv(window1, SCREEN_PROPERTY_POSITION, rect1);
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_POSITION, rect2);

    // Set Z-order (window1 behind window2)
    screen_set_window_property_iv(window1, SCREEN_PROPERTY_ZORDER, &zorder1);
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_ZORDER, &zorder2);


    screen_set_window_property_pv(window1, SCREEN_PROPERTY_DISPLAY, (void**)display_id);
    screen_set_window_property_pv(window2, SCREEN_PROPERTY_DISPLAY, (void**)display_id);
    // Create window buffers
    //screen_create_window_buffers(window1, 1);
    //screen_create_window_buffers(window2, 1);



    // Get buffers
    screen_buffer_t buf1, buf2;
    //screen_get_window_property_pv(window1, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&buf1);
    //screen_get_window_property_pv(window2, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&buf2);

    // Create window buffers - FIXED: Check return values
    err = screen_create_window_buffers(window1, 1);
    if (err) {
        fprintf(stderr, "ERROR: screen_create_window_buffers for window1 failed: %d\n", err);
        return -1;
    }

    err = screen_create_window_buffers(window2, 1);
    if (err) {
        fprintf(stderr, "ERROR: screen_create_window_buffers for window2 failed: %d\n", err);
        return -1;
    }

    // Get buffers - FIXED: Use correct property and check
    err = screen_get_window_property_pv(window1, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&buf1);
    if (err || buf1 == NULL) {
        fprintf(stderr, "ERROR: Failed to get render buffer for window1: %d\n", err);
        return -1;
    }

    // FIX: Try different property names for buffer access
    err = screen_get_window_property_pv(window2, SCREEN_PROPERTY_RENDER_BUFFERS, (void**)&buf2);
    if (err || buf2 == NULL) {
        // Try alternative property name
        fprintf(stderr, "ERROR: Failed to get render buffer for window2: %d\n", err);
    }
    // Configure touch sensitivity
    // Window1: sensitive (consumes touches)
    screen_set_window_property_iv(window1, SCREEN_PROPERTY_SENSITIVITY, &sensitivity_on);

    // Window2: not sensitive (passes touches through)
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_SENSITIVITY, &sensitivity_off);

    // Set transparency for window2
    screen_set_window_property_iv(window2, SCREEN_PROPERTY_TRANSPARENCY, &transparency);

    // Fill window1 with black
    int black = 0xFF000000;  // ARGB black
    screen_fill(screen_ctx, buf1, &black);

    // Fill window2 with semi-transparent color (e.g., semi-transparent blue)
    int semi_transparent_blue = 0x800000FF;  // ARGB semi-transparent blue
    screen_fill(screen_ctx, buf2, &semi_transparent_blue);

    // Post the buffers to display the windows
    screen_post_window(window1, buf1, 0, NULL, 0);
    screen_post_window(window2, buf2, 0, NULL, 0);

    // Flush the context
    screen_flush_context(screen_ctx, 0);

    printf("Windows created successfully!\n");
    printf("Window 1: 500x500 black at position 100,100 (sensitive to touch)\n");
    printf("Window 2: 1000x1000 semi-transparent (touch insensitive)\n");
    printf("Window 2 will pass all touches through to Window 1\n");
    printf("Press Ctrl+C to exit...\n");

    // Event loop - simplified version without touch-specific events
    screen_event_t event;
    screen_create_event(&event);

    int event_count = 0;

    while (1) {
        int event_type;
        // Wait for event with timeout
        err = screen_get_event(screen_ctx, event, 1000); // 1 second timeout

        if (err == 0) {
            screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, &event_type);

            // Handle different event types
            switch (event_type) {
                case SCREEN_EVENT_CREATE:
                    printf("Event %d: SCREEN_EVENT_CREATE\n", event_count++);
                    break;
                case SCREEN_EVENT_CLOSE:
                    printf("Event %d: SCREEN_EVENT_CLOSE\n", event_count++);
                    break;
                case SCREEN_EVENT_POINTER:
                    {
                        int position[2];
                        screen_get_event_property_iv(event, SCREEN_PROPERTY_POSITION, position);
                        printf("Event %d: SCREEN_EVENT_POINTER at (%d, %d)\n",
                               event_count++, position[0], position[1]);
                    }
                    break;
                case SCREEN_EVENT_KEYBOARD:
                    printf("Event %d: SCREEN_EVENT_KEYBOARD\n", event_count++);
                    break;
                default:
                    printf("Event %d: Unknown event type %d\n", event_count++, event_type);
                    break;
            }
        } else {
            // Timeout occurred, just continue
            printf("No events received (timeout)\n");
        }
    }

    // Cleanup
    screen_destroy_event(event);
    screen_destroy_window(window1);
    screen_destroy_window(window2);
    screen_destroy_context(screen_ctx);

    return EXIT_SUCCESS;
}