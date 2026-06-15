import bpy

sc = bpy.context.scene
# (1) ensure render is not border-cropped
sc.render.use_border = False

# (2) revert positions: faceplate back to z=-0.6, knobs/meter back to z=0
p = bpy.data.objects.get('F76_Background')
if p: p.location.z = -0.6
for o in sc.objects:
    if o.name.startswith('F76_knob') or o.name == 'F76_meters':
        o.location.z = 0.0

# (3) lay the screen parts in a tidy tray just LEFT of the faceplate
#     (plate left edge ~ -8.1), so they can be grabbed and placed.
screens = [o for o in sc.objects if o.name.startswith(('Botom_InfoLine_Housing', 'Screen_plate'))]
cols = 6
x0, dx = -10.5, -2.0      # columns march left
y0, dy = 5.0, -1.7        # rows march down
for i, o in enumerate(sorted(screens, key=lambda o: o.name)):
    c, r = i % cols, i // cols
    o.location = (x0 + c * dx, y0 + r * dy, 0.0)
    o.rotation_euler = (0.0, 0.0, 0.0)

bpy.ops.wm.save_mainfile()
print("border off; faceplate z=-0.6; knobs/meter z=0; tray of %d screens left of plate" % len(screens))
