#include <drivers/video/vga_io.h>
#include <drivers/video/vga_modes.h>
#include <drivers/video/vga_palletes.h>

// 80x25x16 text
#define mode_80x25x16_text_misc 0x67

static const uint8_t mode_80x25x16_text_seq[][2] = {
    {VGA_SEQ_RESET, 0x03},
    {VGA_SEQ_CLOCKING, 0x00},
    {VGA_SEQ_MAP_MASK, 0x03},
    {VGA_SEQ_CHAR_MAP_SEL, 0x3a},
    {VGA_SEQ_MEM_MODE, 0x02}
};

static const uint8_t mode_80x25x16_text_crtc[][2] = {
    {VGA_CRTC_HORIZONTAL_TOTAL, 0x5f},
    {VGA_CRTC_HORIZONTAL_DISPLAY_END, 0x4f},
    {VGA_CRTC_HORIZONTAL_BLANK_START, 0x50},
    {VGA_CRTC_HORIZONTAL_BLANK_END, 0x82},
    {VGA_CRTC_HORIZONTAL_RETRACE_START, 0x55},
    {VGA_CRTC_HORIZONTAL_RETRACE_END, 0x81},
    {VGA_CRTC_VERTICAL_TOTAL, 0xbf},
    {VGA_CRTC_OVERFLOW, 0x1f},
    {VGA_CRTC_PRESET_ROW_SCAN, 0x00},
    {VGA_CRTC_MAXIMUM_SCAN_LINE, 0x4f},
    {VGA_CRTC_VERTICAL_RETRACE_START, 0x9c},
    {VGA_CRTC_VERTICAL_RETRACE_END, 0x8e},
    {VGA_CRTC_VERTICAL_DISPLAY_END, 0x8f},
    {VGA_CRTC_OFFSET, 0x28},
    {VGA_CRTC_UNDERLINE_LOCATION, 0x1f},
    {VGA_CRTC_VERTICAL_BLANKING_START, 0x96},
    {VGA_CRTC_VERTICAL_BLANKING_END, 0xb9},
    {VGA_CRTC_MODE_CONTROL, 0xa3}
};

static const uint8_t mode_80x25x16_text_gfx[][2] = {
    {VGA_GFX_SET_RESET, 0x00},
    {VGA_GFX_EN_SET_RESET, 0x00},
    {VGA_GFX_COLOR_CMP, 0x00},
    {VGA_GFX_DATA_ROTATE, 0x00},
    {VGA_GFX_READ_MAP_SEL, 0x00},
    {VGA_GFX_MODE, 0x10},
    {VGA_GFX_MISC, 0x0e},
    {VGA_GFX_CDC, 0x00},
    {VGA_GFX_BIT_MASK, 0xff}
};

static const uint8_t mode_80x25x16_text_attr[][2] = {
    {0x00, 0x00},
    {0x01, 0x01},
    {0x02, 0x02},
    {0x03, 0x03},
    {0x04, 0x04},
    {0x05, 0x05},
    {0x06, 0x14},
    {0x07, 0x07},
    {0x08, 0x38},
    {0x09, 0x39},
    {0x0a, 0x3a},
    {0x0b, 0x3b},
    {0x0c, 0x3c},
    {0x0d, 0x3d},
    {0x0e, 0x3e},
    {0x0f, 0x3f},
    {VGA_AC_MODE_CONTROL, 0x0c},
    {VGA_AC_OVERSCAN_COLOR, 0x00},
    {VGA_AC_COLOR_PLANE_ENABLE, 0x0f},
    {VGA_AC_HORIZONTAL_PIXEL_PAN, 0x08},
    {VGA_AC_COLOR_SELECT, 0x00},
};

// 320x200x256 linear
#define mode_320x200x256_linear_misc 0x63

static const uint8_t mode_320x200x256_linear_seq[][2] = {
    {VGA_SEQ_RESET, 0x03},
    {VGA_SEQ_CLOCKING, 0x01},
    {VGA_SEQ_MAP_MASK, 0x0f},
    {VGA_SEQ_CHAR_MAP_SEL, 0x00},
    {VGA_SEQ_MEM_MODE, 0x0e}
};

static const uint8_t mode_320x200x256_linear_crtc[][2] = {
    {VGA_CRTC_HORIZONTAL_TOTAL, 0x5f},
    {VGA_CRTC_HORIZONTAL_DISPLAY_END, 0x4f},
    {VGA_CRTC_HORIZONTAL_BLANK_START, 0x50},
    {VGA_CRTC_HORIZONTAL_BLANK_END, 0x82},
    {VGA_CRTC_HORIZONTAL_RETRACE_START, 0x54},
    {VGA_CRTC_HORIZONTAL_RETRACE_END, 0x80},
    {VGA_CRTC_VERTICAL_TOTAL, 0xbf},
    {VGA_CRTC_OVERFLOW, 0x1f},
    {VGA_CRTC_PRESET_ROW_SCAN, 0x00},
    {VGA_CRTC_MAXIMUM_SCAN_LINE, 0x4f},
    {VGA_CRTC_VERTICAL_RETRACE_START, 0x9c},
    {VGA_CRTC_VERTICAL_RETRACE_END, 0x8e},
    {VGA_CRTC_VERTICAL_DISPLAY_END, 0x8f},
    {VGA_CRTC_OFFSET, 0x28},
    {VGA_CRTC_UNDERLINE_LOCATION, 0x40},
    {VGA_CRTC_VERTICAL_BLANKING_START, 0x96},
    {VGA_CRTC_VERTICAL_BLANKING_END, 0xb9},
    {VGA_CRTC_MODE_CONTROL, 0xa3}
};

static const uint8_t mode_320x200x256_linear_gfx[][2] = {
    {VGA_GFX_SET_RESET, 0x00},
    {VGA_GFX_EN_SET_RESET, 0x00},
    {VGA_GFX_COLOR_CMP, 0x00},
    {VGA_GFX_DATA_ROTATE, 0x00},
    {VGA_GFX_READ_MAP_SEL, 0x00},
    {VGA_GFX_MODE, 0x40},
    {VGA_GFX_MISC, 0x05},
    {VGA_GFX_CDC, 0x5f},
    {VGA_GFX_BIT_MASK, 0x4f}
};

static const uint8_t mode_320x200x256_linear_attr[][2] = {
    {0x00, 0x00},
    {0x01, 0x01},
    {0x02, 0x02},
    {0x03, 0x03},
    {0x04, 0x04},
    {0x05, 0x05},
    {0x06, 0x14},
    {0x07, 0x07},
    {0x08, 0x08},
    {0x09, 0x09},
    {0x0a, 0x0a},
    {0x0b, 0x0b},
    {0x0c, 0x0c},
    {0x0d, 0x0d},
    {0x0e, 0x0e},
    {0x0f, 0x0f},
    {VGA_AC_MODE_CONTROL, 0x41},
    {VGA_AC_OVERSCAN_COLOR, 0x00},
    {VGA_AC_COLOR_PLANE_ENABLE, 0x0f},
    {VGA_AC_HORIZONTAL_PIXEL_PAN, 0x00},
    {VGA_AC_COLOR_SELECT, 0x00},
};

// Struct definitions here
struct vga_mode vga_mode_80x25x16_text = {
    .misc = mode_80x25x16_text_misc,
    .seq = mode_80x25x16_text_seq,
    .seq_len = sizeof(mode_80x25x16_text_seq) / 2,
    .crtc = mode_80x25x16_text_crtc,
    .crtc_len = sizeof(mode_80x25x16_text_crtc) / 2,
    .gfx = mode_80x25x16_text_gfx,
    .gfx_len = sizeof(mode_80x25x16_text_gfx) / 2,
    .attr = mode_80x25x16_text_attr,
    .attr_len = sizeof(mode_80x25x16_text_attr) / 2,
    .pallete = vga_pallete_16,
    .pallete_entries = vga_pallete_16_len
};

struct vga_mode vga_mode_320x200x256_linear = {
    .misc = mode_320x200x256_linear_misc,
    .seq = mode_320x200x256_linear_seq,
    .seq_len = sizeof(mode_320x200x256_linear_seq) / 2,
    .crtc = mode_320x200x256_linear_crtc,
    .crtc_len = sizeof(mode_320x200x256_linear_crtc) / 2,
    .gfx = mode_320x200x256_linear_gfx,
    .gfx_len = sizeof(mode_320x200x256_linear_gfx) / 2,
    .attr = mode_320x200x256_linear_attr,
    .attr_len = sizeof(mode_320x200x256_linear_attr) / 2,
};
