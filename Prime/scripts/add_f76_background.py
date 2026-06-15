import bpy

# Adds a background faceplate plate to Filter 76 Prime: a panel plane covering the
# full landscape canvas, behind the controls, using the faceplate material borrowed
# from the VST MOD Faceplate asset. (Placeholder background for the blockout.)
LIBHW = r"C:\Users\ninov\OneDrive\Documents\AlteredAudio\The VST MOD\VST_MOD_Hardware.blend"
PPU, W, H = 182.3, 2800.0, 1800.0

# borrow the faceplate material by appending the Faceplate object
with bpy.data.libraries.load(LIBHW, link=False) as (src, dst):
    dst.objects = [n for n in ["Faceplate"] if n in src.objects]
fp = next((o for o in dst.objects if o), None)
mat = fp.data.materials[0] if (fp and fp.data and len(fp.data.materials)) else None

# full-canvas background plane, just behind the controls
bpy.ops.mesh.primitive_plane_add(size=2.0, location=(0.0, 0.0, -0.6))
plane = bpy.context.active_object
plane.name = "F76_Background"
plane.scale = (W / PPU / 2.0, H / PPU / 2.0, 1.0)
plane.data.materials.clear()
if mat is not None:
    plane.data.materials.append(mat)
else:
    m = bpy.data.materials.new("F76_Panel"); m.use_nodes = True
    b = m.node_tree.nodes.get("Principled BSDF")
    if b: b.inputs["Base Color"].default_value = (0.62, 0.50, 0.32, 1.0)
    plane.data.materials.append(m)

# drop the borrowed object (keep only its material on the plane)
if fp is not None:
    bpy.data.objects.remove(fp, do_unlink=True)

bpy.ops.wm.save_mainfile()
print("background plate added; material:", (mat.name if mat else "F76_Panel"))
