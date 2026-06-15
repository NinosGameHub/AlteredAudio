import bpy, mathutils

sc = bpy.context.scene
vl = bpy.context.view_layer

# (1) make the selected collection instance(s) real -> separate objects
sel = [o for o in bpy.context.selected_objects if o.instance_type == 'COLLECTION']
made = []
if sel:
    for o in vl.objects:
        try: o.select_set(False)
        except Exception: pass
    for o in sel: o.select_set(True)
    vl.objects.active = sel[0]
    win = bpy.context.window
    area = next((a for a in win.screen.areas if a.type == 'VIEW_3D'), None) if win and win.screen else None
    if area:
        region = next((r for r in area.regions if r.type == 'WINDOW'), None)
        with bpy.context.temp_override(window=win, area=area, region=region,
                                       selected_objects=sel, active_object=sel[0]):
            bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)
    else:
        bpy.ops.object.duplicates_make_real(use_base_parent=False, use_hierarchy=True)
    for o in list(sel):
        made.append(o.name)
        if o.instance_type == 'COLLECTION':
            try: bpy.data.objects.remove(o, do_unlink=True)
            except Exception: pass

# (2) camera fills the faceplate exactly (no margin)
p = bpy.data.objects.get('F76_Background')
bb = [p.matrix_world @ mathutils.Vector(v) for v in p.bound_box]
xs = [v.x for v in bb]; ys = [v.y for v in bb]
cx = (min(xs) + max(xs)) / 2.0; cy = (min(ys) + max(ys)) / 2.0
pw = max(xs) - min(xs); ph = max(ys) - min(ys)
cam = bpy.data.objects['Camera']
cam.location.x = cx; cam.location.y = cy
asp = sc.render.resolution_x / sc.render.resolution_y
cam.data.ortho_scale = max(pw, ph * asp)   # fill, no margin

bpy.ops.wm.save_mainfile()
print("separated:", made, "| camera fills plate, ortho=%.2f" % cam.data.ortho_scale)
