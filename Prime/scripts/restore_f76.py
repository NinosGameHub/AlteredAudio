import bpy

# (1) restore the screen parts (re-instance VSTMOD_InfoLine x3 and make real, same as
#     before), (2) faceplate to z=0 ground level, (3) lift knobs+meter on top of it.
LIBHW = r"C:\Users\ninov\OneDrive\Documents\AlteredAudio\The VST MOD\VST_MOD_Hardware.blend"
sc = bpy.context.scene
vl = bpy.context.view_layer
PPU, W, H = 182.3, 2800.0, 1800.0
def world(cx, cy): return ((2*cx - W/2)/PPU, (H/2 - 2*cy)/PPU, 0.0)

coll = bpy.data.collections.get('VSTMOD_InfoLine')
if coll is None:
    with bpy.data.libraries.load(LIBHW, link=False) as (src, dst):
        dst.collections = [n for n in ['VSTMOD_InfoLine'] if n in src.collections]
    coll = bpy.data.collections.get('VSTMOD_InfoLine')

insts = []
for tag, (cx, cy, scl) in {'display': (611, 248, 2.2), 'lfo': (578, 740, 0.7), 'env': (1050, 723, 0.6)}.items():
    o = bpy.data.objects.new('F76_screen_' + tag, None)
    o.instance_type = 'COLLECTION'; o.instance_collection = coll
    o.location = world(cx, cy); o.scale = (scl, scl, scl); o.empty_display_size = 0.1
    sc.collection.objects.link(o); insts.append(o)

for o in vl.objects:
    try: o.select_set(False)
    except Exception: pass
for o in insts: o.select_set(True)
vl.objects.active = insts[0]
win = bpy.context.window
area = next((a for a in win.screen.areas if a.type == 'VIEW_3D'), None) if win and win.screen else None
if area:
    region = next((r for r in area.regions if r.type == 'WINDOW'), None)
    with bpy.context.temp_override(window=win, area=area, region=region,
                                   selected_objects=insts, active_object=insts[0]):
        bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)
else:
    bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)
for o in list(insts):
    if o.instance_type == 'COLLECTION':
        try: bpy.data.objects.remove(o, do_unlink=True)
        except Exception: pass

# (2) faceplate at ground level z=0
p = bpy.data.objects.get('F76_Background')
if p: p.location.z = 0.0
# (3) knobs + meter on top of the plate
for o in sc.objects:
    if o.name.startswith('F76_knob') or o.name == 'F76_meters':
        o.location.z = 0.3

bpy.ops.wm.save_mainfile()
print("restored screens; faceplate z=0; knobs/meter z=0.3")
