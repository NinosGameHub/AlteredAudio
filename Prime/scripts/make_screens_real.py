import bpy

# Converts the F76_screen_* collection instances (each = the whole VSTMOD_InfoLine
# strip) into real, independent objects so each housing/plate can be moved alone.
sc = bpy.context.scene
vl = bpy.context.view_layer

for o in list(vl.objects):
    try: o.select_set(False)
    except Exception: pass

targets = [o for o in sc.objects if o.name.startswith('F76_screen')]
before = set(o.name for o in sc.objects)

if targets:
    for o in targets:
        o.select_set(True)
    vl.objects.active = targets[0]

    win = bpy.context.window
    area = next((a for a in win.screen.areas if a.type == 'VIEW_3D'), None) if win and win.screen else None
    if area:
        region = next((r for r in area.regions if r.type == 'WINDOW'), None)
        with bpy.context.temp_override(window=win, area=area, region=region,
                                       selected_objects=targets, active_object=targets[0]):
            bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)
    else:
        bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)

    # remove the now-redundant instance empties
    for o in targets:
        if o.instance_type == 'COLLECTION':
            try: bpy.data.objects.remove(o, do_unlink=True)
            except Exception: pass

new = [n for n in (set(o.name for o in sc.objects) - before)]
bpy.ops.wm.save_mainfile()
print("made real -> %d new objects" % len(new))
print(new[:20])
