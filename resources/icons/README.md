# Application Icons

This directory contains the application icons for the SCV Project.

## Current Icons

- **app_icon.svg**: Vector icon (scalable, used for display)
- **app_icon.png**: Raster icon (placeholder - replace with your own 256x256 PNG)
- **app_icon.ico**: Windows icon (placeholder - replace with your own multi-size ICO)

## How to Replace Icons

### For PNG Icon:
1. Create or obtain a PNG image (recommended size: 256x256 pixels or larger)
2. Replace `app_icon.png` with your new icon
3. Keep the same filename or update `resources.qrc` accordingly

### For Windows ICO Icon:
1. Create a .ico file with multiple sizes (16x16, 32x32, 48x48, 256x256)
2. You can use online tools like:
   - https://www.icoconverter.com/
   - https://convertio.co/png-ico/
3. Replace `app_icon.ico` with your new icon

### For SVG Icon:
1. Edit `app_icon.svg` with any vector graphics editor (Inkscape, Adobe Illustrator, etc.)
2. Or replace it with your own SVG file

## Recommended Icon Sizes

- **Windows**: 256x256 pixels (or .ico with multiple sizes)
- **macOS**: 512x512 pixels or 1024x1024 pixels for Retina displays
- **Linux**: 256x256 pixels or scalable SVG

## After Replacing Icons

After replacing any icon files:
1. Rebuild the project to include the new resources
2. The changes will be reflected in the application window and taskbar

