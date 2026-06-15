import bpy, os

# Sets up the Filter 76 Prime starting scene from the VST MOD template: landscape
# camera for the 1400x900 design (same 182.3 px/world-unit -> render 2800x1800),
# then blocks out the VST MOD assets at the procedural-UI control positions
# (knobs for knobs, a meter column, screens for the readout/scope/display zones).
LIBDIR   = r"C:\Users\ninov\OneDrive\Documents\AlteredAudio\The VST MOD"
TEMPLATE = os.path.join(LIBDIR, "Template.blend")
LIBHW    = os.path.join(LIBDIR, "VST_MOD_Hardware.blend")
OUTDIR   = r"C:\Dev\Blender\Filter 76 Prime"
os.makedirs(OUTDIR, exist_ok=True)
OUT      = os.path.join(OUTDIR, "Filter76_Prime.blend")

bpy.ops.wm.open_mainfile(filepath=TEMPLATE)
sc = bpy.context.scene

# landscape framing, same px/world-unit as Gain (182.3 @2x)
PPU, W, H = 182.3, 2800.0, 1800.0
sc.render.resolution_x = int(W)
sc.render.resolution_y = int(H)
cam = bpy.data.objects.get('Camera')
if cam and cam.data.type == 'ORTHO':
    cam.data.ortho_scale = W / PPU      # ~15.36

# append element collections from the asset library
want = ["VSTMOD_Knob", "VSTMOD_Meter", "VSTMOD_InfoLine"]
with bpy.data.libraries.load(LIBHW, link=False) as (src, dst):
    dst.collections = [n for n in want if n in src.collections]
appended = {c.name: c for c in dst.collections}

def world(cx, cy):                       # canvas(1400x900 px) -> world
    return ((2.0 * cx - W / 2) / PPU, (H / 2 - 2.0 * cy) / PPU, 0.0)

def place(coll, cx, cy, scale, tag):
    c = appended.get(coll)
    if not c:
        return
    o = bpy.data.objects.new("F76_" + tag, None)
    o.instance_type = 'COLLECTION'
    o.instance_collection = c
    o.location = world(cx, cy)
    o.scale = (scale, scale, scale)
    o.empty_display_size = 0.1
    sc.collection.objects.link(o)

HERO, SMALL = 0.52, 0.30
# filter hero knobs: freq res drive mix out  (+ gain)
for i, cx in enumerate([275, 465, 655, 845, 1035]):
    place("VSTMOD_Knob", cx, 532, HERO, "knob_hero_%d" % i)
place("VSTMOD_Knob", 1175, 532, SMALL, "knob_gain")
# mod amount + LFO rate/depth/phase + env atk/rel/sens
for i, (cx, cy) in enumerate([(383, 740), (732, 748), (808, 748), (884, 748),
                              (1188, 748), (1264, 748), (1340, 748)]):
    place("VSTMOD_Knob", cx, cy, SMALL, "knob_mod_%d" % i)
# meter column (right of the display)
place("VSTMOD_Meter", 1300, 257, 1.15, "meters")
# screens: big response display + LFO scope + ENV scope (InfoLine as placeholder)
place("VSTMOD_InfoLine", 611, 248, 2.2, "screen_display")
place("VSTMOD_InfoLine", 578, 740, 0.7, "screen_lfo")
place("VSTMOD_InfoLine", 1050, 723, 0.6, "screen_env")

bpy.ops.wm.save_as_mainfile(filepath=OUT)
print("SAVED:", OUT, "| placed knobs+meter+screens")
