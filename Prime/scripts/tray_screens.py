import bpy, mathutils

# Creates real, independent screen parts (the VSTMOD_InfoLine housings + plates)
# and lays them in a tidy grid tray just LEFT of the faceplate, so each can be
# grabbed and placed. Geometry (not just origin) is centered on each grid slot.
sc = bpy.context.scene
coll = bpy.data.collections.get('VSTMOD_InfoLine')
src = list(coll.objects) if coll else []

tray = []
for o in src:
    cp = o.copy()
    if o.data: cp.data = o.data.copy()      # single-user (independent)
    cp.scale = (1.0, 1.0, 1.0)
    cp.rotation_euler = (0.0, 0.0, 0.0)
    sc.collection.objects.link(cp)
    tray.append(cp)

cols = 4
x0, dx = -10.5, -2.2      # columns march left of the plate
y0, dy = 4.0, -2.0        # rows march down
for i, cp in enumerate(tray):
    c, r = i % cols, i // cols
    lbc = sum((mathutils.Vector(v) for v in cp.bound_box), mathutils.Vector()) / 8.0
    cp.location = (x0 + c * dx - lbc.x, y0 + r * dy - lbc.y, 0.2 - lbc.z)

bpy.ops.wm.save_mainfile()
print("created %d screen parts in a tray left of the faceplate" % len(tray))
