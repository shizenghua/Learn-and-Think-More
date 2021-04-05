# Porting from IDAPython 6.x-7.3, to 7.4

url：https://www.hex-rays.com/products/ida/support/ida74_idapython_no_bc695_porting_guide.shtml

## Intended audience

IDAPython developers

### The problem

IDA 7.4 [turns off IDA 6.x API backwards-compatibility by default](https://www.hex-rays.com/products/ida/support/ida74_idapython_no_bc695.shtml).

Although there is a trivial way of turning 6.x backwards-compatibility back on, this should be considered a temporary measure, until the code is ported to the newer APIs (that have started shipping with IDA 7.0, back in 2017.)

### Supporting IDA 7.x

Note that the new APIs have been baked in IDAPython since IDA 7.0, meaning that by porting existing IDAPython code according to this guide, you will not just support IDA 7.4: the ported code will also work in IDA 7.3, 7.2, 7.1 and 7.0.

### This is a complementary guide

A [general-purpose porting guide](https://www.hex-rays.com/products/ida/7.0/docs/api70_porting_guide.shtml) shipped at the time, which covers a great deal of the changes and is enough to port C/C++ code.

Alas, we now see that it is insufficient when it comes to accompanying developers in the task of porting their IDAPython code to the newer APIs, simply because IDAPython has some specific concepts & constructs, that require special attention.

That being said, this very guide should be considered as a *complement* to the original guide, and not as a *replacement*.

### The guide

Note: that all qualified names below use their originating IDAPython module's name (e.g., `ida_kernwin`) instead of the 'umbrella' `idaapi` module.

The following types have been moved/renamed:



| Before                                                       | After                                                        | Notes                                                        |
| ------------------------------------------------------------ | ------------------------------------------------------------ | ------------------------------------------------------------ |
| `ida_kernwin.Choose2`                                        | `ida_kernwin.Choose`                                         |                                                              |
| `ida_ua.insn_t.Operands`                                     | `ida_ua.insn_t.ops`                                          |                                                              |
| `ida_kernwin.Choose2.OnSelectLine`                           | `ida_kernwin.Choose.OnSelectLine`                            | if the chooser is `CH_MULTI`, will receive (and must return) a list of selected indices, instead of just 1 index |
| `ida_kernwin.Choose2.OnDeleteLine`                           | ` class my_choose_t(ida_kernwin.Choose):     [...]     def OnDeleteLine(self, indices):         new_items = []         for idx, item in enumerate(self.items):             if idx not in indices:                 new_items.append(item)         self.items = new_items         return [Choose.ALL_CHANGED] + indices ` | if the chooser is `CH_MULTI`, it should return `Choose.ALL_CHANGED` as part of the returned list, if a refresh is desired |
| `ida_bytes.data_type_t.__init__`                             | `ida_bytes.data_type_t.__init__`                             | arguments must be passed sequentially, not by keyword        |
| `ida_idp.IDP_Hooks.auto_queue_empty`                         | `ida_idp.IDP_Hooks.ev_auto_queue_empty`                      |                                                              |
| `ida_kernwin.AST_ENABLE_FOR_FORM`                            | `ida_kernwin.AST_ENABLE_FOR_WIDGET`                          |                                                              |
| `ida_kernwin.AST_DISABLE_FOR_FORM`                           | `ida_kernwin.AST_DISABLE_FOR_WIDGET`                         |                                                              |
| `ida_kernwin.CB_CLOSE_IDB`                                   | `ida_kernwin.CB_INVISIBLE`                                   |                                                              |
| `ida_kernwin.chtype_generic2`                                | `ida_kernwin.chtype_generic`                                 |                                                              |
| `ida_kernwin.chtype_segreg`                                  | `ida_kernwin.chtype_srcp`                                    |                                                              |
| `ida_kernwin.close_tform`                                    | `ida_kernwin.close_widget`                                   |                                                              |
| `ida_kernwin.find_tform`                                     | `ida_kernwin.find_widget`                                    |                                                              |
| `ida_kernwin.get_current_tform`                              | `ida_kernwin.get_current_widget`                             |                                                              |
| `ida_kernwin.get_highlighted_identifier()`                   | ` v = ida_kernwin.get_current_viewer() thing = ida_kernwin.get_highlight(v) if thing and thing[1]:     identifier = thing[0] ` |                                                              |
| `ida_kernwin.get_tform_title`                                | `ida_kernwin.get_widget_title`                               |                                                              |
| `ida_kernwin.get_tform_type`                                 | `ida_kernwin.get_widget_type`                                |                                                              |
| `ida_kernwin.is_chooser_tform`                               | `ida_kernwin.is_chooser_widget`                              |                                                              |
| `ida_kernwin.open_tform`                                     | `ida_kernwin.display_widget`                                 |                                                              |
| `ida_kernwin.pyscv_get_tcustom_control`                      | `ida_kernwin.pyscv_get_widget`                               |                                                              |
| `ida_kernwin.pyscv_get_tform`                                | `ida_kernwin.pyscv_get_widget`                               |                                                              |
| `ida_kernwin.switchto_tform`                                 | `ida_kernwin.activate_widget`                                |                                                              |
| `ida_kernwin.umsg`                                           | `ida_kernwin.msg`                                            |                                                              |
| `ida_kernwin.UI_Hooks.tform_visible`                         | `ida_kernwin.UI_Hooks.widget_visible`                        |                                                              |
| `ida_kernwin.UI_Hooks.tform_invisible`                       | `ida_kernwin.UI_Hooks.widget_invisible`                      |                                                              |
| `ida_kernwin.UI_Hooks.populating_tform_popup`                | `ida_kernwin.UI_Hooks.populating_widget_popup`               |                                                              |
| `ida_kernwin.UI_Hooks.finish_populating_tform_popup`         | `ida_kernwin.UI_Hooks.finish_populating_widget_popup`        |                                                              |
| `ida_kernwin.UI_Hooks.current_tform_changed`                 | `ida_kernwin.UI_Hooks.current_widget_changed`                |                                                              |
| `ida_kernwin.AskUsingForm`                                   | `ida_kernwin.ask_form`                                       |                                                              |
| `ida_kernwin.HIST_ADDR`                                      | `0`                                                          |                                                              |
| `ida_kernwin.HIST_NUM`                                       | `0`                                                          |                                                              |
| `ida_kernwin.KERNEL_VERSION_MAGIC1`                          | `0`                                                          |                                                              |
| `ida_kernwin.KERNEL_VERSION_MAGIC2`                          | `0`                                                          |                                                              |
| `ida_kernwin.OpenForm`                                       | `ida_kernwin.open_form`                                      |                                                              |
| `ida_kernwin._askaddr`                                       | `_ida_kernwin._ask_addr`                                     |                                                              |
| `ida_kernwin._asklong`                                       | `_ida_kernwin._ask_long`                                     |                                                              |
| `ida_kernwin._askseg`                                        | `_ida_kernwin._ask_seg`                                      |                                                              |
| `ida_kernwin.askaddr`                                        | `ida_kernwin.ask_addr`                                       |                                                              |
| `ida_kernwin.askbuttons_c`                                   | `ida_kernwin.ask_buttons`                                    |                                                              |
| `ida_kernwin.askfile_c`                                      | `ida_kernwin.ask_file`                                       |                                                              |
| `ida_kernwin.askfile2_c(forsave, defdir, filters, fmt)`      | ` if filters:     fmt = "FILTER %s\n%s" % (filters, fmt) ask_file(forsave, defdir, fmt) ` |                                                              |
| `ida_kernwin.askident`                                       | `ida_kernwin.ask_ident`                                      |                                                              |
| `ida_kernwin.asklong`                                        | `ida_kernwin.ask_long`                                       |                                                              |
| `ida_kernwin.askqstr(defval, fmt)`                           | `ida_kernwin.ask_str(defval, 0, fmt)`                        |                                                              |
| `ida_kernwin.askseg`                                         | `ida_kernwin.ask_seg`                                        |                                                              |
| `ida_kernwin.askstr(hist, defval, fmt)`                      | `ida_kernwin.ask_str(defval, hist, fmt)`                     |                                                              |
| `ida_kernwin.asktext`                                        | `ida_kernwin.ask_text`                                       |                                                              |
| `ida_kernwin.askyn_c`                                        | `ida_kernwin.ask_yn`                                         |                                                              |
| `ida_kernwin.choose2_activate`                               | `ida_kernwin.choose_activate`                                |                                                              |
| `ida_kernwin.choose2_close`                                  | `ida_kernwin.choose_close`                                   |                                                              |
| `ida_kernwin.choose2_find`                                   | `ida_kernwin.choose_find`                                    |                                                              |
| `ida_kernwin.choose2_get_embedded_selection`                 | `ida_kernwin.lambda *args: None`                             |                                                              |
| `ida_kernwin.choose2_refresh`                                | `ida_kernwin.choose_refresh`                                 |                                                              |
| `ida_kernwin.clearBreak`                                     | `ida_kernwin.clr_cancelled`                                  |                                                              |
| `ida_kernwin.py_get_AskUsingForm`                            | `ida_kernwin.py_get_ask_form`                                |                                                              |
| `ida_kernwin.py_get_OpenForm`                                | `ida_kernwin.py_get_open_form`                               |                                                              |
| `ida_kernwin.setBreak`                                       | `ida_kernwin.set_cancelled`                                  |                                                              |
| `ida_kernwin.wasBreak`                                       | `ida_kernwin.user_cancelled`                                 |                                                              |
| `ida_kernwin.refresh_lists`                                  | `ida_kernwin.refresh_choosers`                               |                                                              |
| `ida_range.range_t.startEA`                                  | `ida_range.range_t.start_ea`                                 |                                                              |
| `ida_range.range_t.endEA`                                    | `ida_range.range_t.end_ea`                                   |                                                              |
| `ida_funcs.func_t.startEA`                                   | `ida_funcs.func_t.start_ea`                                  |                                                              |
| `ida_funcs.func_t.endEA`                                     | `ida_funcs.func_t.end_ea`                                    |                                                              |
| `ida_segment.segment_t.startEA`                              | `ida_segment.segment_t.start_ea`                             |                                                              |
| `ida_segment.segment_t.endEA`                                | `ida_segment.segment_t.end_ea`                               |                                                              |
| `ida_kernwin.PluginForm.FORM_MDI`                            | `ida_kernwin.PluginForm.WOPN_MDI`                            |                                                              |
| `ida_kernwin.PluginForm.FORM_TAB`                            | `ida_kernwin.PluginForm.WOPN_TAB`                            |                                                              |
| `ida_kernwin.PluginForm.FORM_RESTORE`                        | `ida_kernwin.PluginForm.WOPN_RESTORE`                        |                                                              |
| `ida_kernwin.PluginForm.FORM_ONTOP`                          | `ida_kernwin.PluginForm.WOPN_ONTOP`                          |                                                              |
| `ida_kernwin.PluginForm.FORM_MENU`                           | `ida_kernwin.PluginForm.WOPN_MENU`                           |                                                              |
| `ida_kernwin.PluginForm.FORM_CENTERED`                       | `ida_kernwin.PluginForm.WOPN_CENTERED`                       |                                                              |
| `ida_kernwin.PluginForm.FORM_PERSIST`                        | `ida_kernwin.PluginForm.WOPN_PERSIST`                        |                                                              |
| `ida_kernwin.PluginForm.FORM_SAVE`                           | `ida_kernwin.PluginForm.WCLS_SAVE`                           |                                                              |
| `ida_kernwin.PluginForm.FORM_NO_CONTEXT`                     | `ida_kernwin.PluginForm.WCLS_NO_CONTEXT`                     |                                                              |
| `ida_kernwin.PluginForm.FORM_DONT_SAVE_SIZE`                 | `ida_kernwin.PluginForm.WCLS_DONT_SAVE_SIZE`                 |                                                              |
| `ida_kernwin.PluginForm.FORM_CLOSE_LATER`                    | `ida_kernwin.PluginForm.WCLS_CLOSE_LATER`                    |                                                              |
| `ida_lines.add_long_cmt`                                     | `ida_lines.add_extra_cmt`                                    |                                                              |
| `ida_lines.describe`                                         | `ida_lines.add_extra_line`                                   |                                                              |
| `ida_search.find_void`                                       | `ida_search.find_suspop`                                     |                                                              |
| `ida_srarea`                                                 | `ida_segregs`                                                |                                                              |
| `ida_srarea.SetDefaultRegisterValue`                         | `ida_segregs.set_default_sreg_value`                         |                                                              |
| `ida_srarea.copy_srareas`                                    | `ida_segregs.copy_sreg_ranges`                               |                                                              |
| `ida_srarea.del_srarea`                                      | `ida_segregs.del_sreg_range`                                 |                                                              |
| `ida_srarea.getSR`                                           | `ida_segregs.get_sreg`                                       |                                                              |
| `ida_srarea.get_prev_srarea`                                 | `ida_segregs.get_prev_sreg_range`                            |                                                              |
| `ida_srarea.get_srarea2`                                     | `ida_segregs.get_sreg_range`                                 |                                                              |
| `ida_srarea.get_srarea_num`                                  | `ida_segregs.get_sreg_range_num`                             |                                                              |
| `ida_srarea.get_srareas_qty2`                                | `ida_segregs.get_sreg_range_qty`                             |                                                              |
| `ida_srarea.getn_srarea2`                                    | `ida_segregs.getn_sreg_range`                                |                                                              |
| `ida_srarea.is_segreg_locked`                                | `False`                                                      |                                                              |
| `ida_srarea.segreg_area_t`                                   | `ida_segregs.sreg_range_t`                                   |                                                              |
| `ida_srarea.splitSRarea1`                                    | `ida_segregs.split_sreg_range`                               |                                                              |
| `ida_srarea.split_srarea`                                    | `ida_segregs.split_sreg_range`                               |                                                              |
| `ida_srarea.get_segreg`                                      | `ida_segregs.get_sreg`                                       |                                                              |
| `ida_srarea.set_default_segreg_value`                        | `ida_segregs.set_default_sreg_value`                         |                                                              |
| `ida_idd.PROCESS_NO_THREAD`                                  | `ida_idd.NO_THREAD`                                          |                                                              |
| `ida_pro.strlwr`                                             | `str(s).lower()`                                             |                                                              |
| `ida_pro.strupr`                                             | `str(s).upper()`                                             |                                                              |
| `ida_segment.CSS_NOAREA`                                     | `ida_segment.CSS_NORANGE`                                    |                                                              |
| `ida_segment.SEGDEL_KEEP`                                    | `ida_segment.SEGMOD_KEEP`                                    |                                                              |
| `ida_segment.SEGDEL_KEEP0`                                   | `ida_segment.SEGMOD_KEEP0`                                   |                                                              |
| `ida_segment.SEGDEL_PERM`                                    | `ida_segment.SEGMOD_KILL`                                    |                                                              |
| `ida_segment.SEGDEL_SILENT`                                  | `ida_segment.SEGMOD_SILENT`                                  |                                                              |
| `ida_segment.ask_selected`                                   | `ida_segment.sel2para`                                       |                                                              |
| `ida_segment.del_segment_cmt(s, rpt)`                        | `ida_segment.set_segment_cmt(s, "", rpt)`                    |                                                              |
| `ida_segment.get_true_segm_name`                             | `ida_segment.get_segm_name`                                  |                                                              |
| `ida_area`                                                   | `ida_range`                                                  |                                                              |
| `ida_area.area_t`                                            | `ida_range.range_t`                                          |                                                              |
| `ida_frame.add_auto_stkpnt2`                                 | `ida_frame.add_auto_stkpnt`                                  |                                                              |
| `ida_frame.get_stkvar(op, v)`                                | `ida_frame.get_stkvar(op, insn, v)`                          |                                                              |
| `ida_frame.get_frame_part(pfn, part, range)`                 | `ida_frame.get_frame_part(range, pfn, part)`                 |                                                              |
| `ida_strlist.refresh_strlist`                                | `ida_strlist.build_strlist`                                  |                                                              |
| `ida_queue`                                                  | `ida_problems`                                               |                                                              |
| `ida_queue.Q_Qnum`                                           | `ida_problems.cvar.PR_END`                                   |                                                              |
| `ida_queue.Q_att`                                            | `ida_problems.cvar.PR_ATTN`                                  |                                                              |
| `ida_queue.Q_badstack`                                       | `ida_problems.cvar.PR_BADSTACK`                              |                                                              |
| `ida_queue.Q_collsn`                                         | `ida_problems.cvar.PR_COLLISION`                             |                                                              |
| `ida_queue.Q_decimp`                                         | `ida_problems.cvar.PR_DECIMP`                                |                                                              |
| `ida_queue.Q_disasm`                                         | `ida_problems.cvar.PR_DISASM`                                |                                                              |
| `ida_queue.Q_final`                                          | `ida_problems.cvar.PR_FINAL`                                 |                                                              |
| `ida_queue.Q_head`                                           | `ida_problems.cvar.PR_HEAD`                                  |                                                              |
| `ida_queue.Q_jumps`                                          | `ida_problems.cvar.PR_JUMP`                                  |                                                              |
| `ida_queue.Q_lines`                                          | `ida_problems.cvar.PR_MANYLINES`                             |                                                              |
| `ida_queue.Q_noBase`                                         | `ida_problems.cvar.PR_NOBASE`                                |                                                              |
| `ida_queue.Q_noComm`                                         | `ida_problems.cvar.PR_NOCMT`                                 |                                                              |
| `ida_queue.Q_noFop`                                          | `ida_problems.cvar.PR_NOFOP`                                 |                                                              |
| `ida_queue.Q_noName`                                         | `ida_problems.cvar.PR_NONAME`                                |                                                              |
| `ida_queue.Q_noRef`                                          | `ida_problems.cvar.PR_NOXREFS`                               |                                                              |
| `ida_queue.Q_noValid`                                        | `ida_problems.cvar.PR_ILLADDR`                               |                                                              |
| `ida_queue.Q_rolled`                                         | `ida_problems.cvar.PR_ROLLED`                                |                                                              |
| `ida_queue.QueueDel`                                         | `ida_problems.forget_problem`                                |                                                              |
| `ida_queue.QueueGetMessage`                                  | `ida_problems.get_problem_desc`                              |                                                              |
| `ida_queue.QueueGetType`                                     | `ida_problems.get_problem`                                   |                                                              |
| `ida_queue.QueueIsPresent`                                   | `ida_problems.is_problem_present`                            |                                                              |
| `ida_queue.QueueSet`                                         | `ida_problems.remember_problem`                              |                                                              |
| `ida_queue.get_long_queue_name(t)`                           | `ida_problems.get_problem_name(t, True)`                     |                                                              |
| `ida_queue.get_short_queue_name(t)`                          | `ida_problems.get_problem_name(t, False)`                    |                                                              |
| `ida_loader.NEF_TIGHT`                                       | `0`                                                          |                                                              |
| `ida_loader.save_database(path, )`                           | `ida_loader.save_database(path, ida_loader.DBFL_KILL)`       |                                                              |
| `ida_loader.save_database_ex`                                | `ida_loader.save_database`                                   |                                                              |
| `ida_loader.MAX_FILE_FORMAT_NAME`                            | `64`                                                         |                                                              |
| `ida_idp.AS_NOTAB`                                           | `0`                                                          |                                                              |
| `ida_idp.CUSTOM_CMD_ITYPE`                                   | `ida_idp.CUSTOM_INSN_ITYPE`                                  |                                                              |
| `ida_idp.InstrIsSet`                                         | `ida_idp.has_insn_feature`                                   |                                                              |
| `ida_idp.NEXTEAS_ANSWER_SIZE`                                | `0`                                                          |                                                              |
| `ida_idp.PR_FULL_HIFXP`                                      | `0`                                                          |                                                              |
| `ida_idp.SETPROC_ALL`                                        | `ida_idp.SETPROC_LOADER_NON_FATAL`                           |                                                              |
| `ida_idp.SETPROC_COMPAT`                                     | `ida_idp.SETPROC_IDB`                                        |                                                              |
| `ida_idp.SETPROC_FATAL`                                      | `ida_idp.SETPROC_LOADER`                                     |                                                              |
| `ida_idp.area_cmt_changed`                                   | `ida_idp.range_cmt_changed`                                  |                                                              |
| `ida_idp.changed_stkpnts`                                    | `ida_idp.stkpnts_changed`                                    |                                                              |
| `ida_idp.changed_struc`                                      | `ida_idp.struc_align_changed`                                |                                                              |
| `ida_idp.changing_area_cmt`                                  | `ida_idp.changing_range_cmt`                                 |                                                              |
| `ida_idp.changing_struc`                                     | `ida_idp.changing_struc_align`                               |                                                              |
| `ida_idp.func_tail_removed`                                  | `ida_idp.func_tail_deleted`                                  |                                                              |
| `ida_idp.get_reg_info2`                                      | `ida_idp.get_reg_info`                                       |                                                              |
| `ida_idp.ph_get_regCodeSreg`                                 | `ida_idp.ph_get_reg_code_sreg`                               |                                                              |
| `ida_idp.ph_get_regDataSreg`                                 | `ida_idp.ph_get_reg_data_sreg`                               |                                                              |
| `ida_idp.ph_get_regFirstSreg`                                | `ida_idp.ph_get_reg_first_sreg`                              |                                                              |
| `ida_idp.ph_get_regLastSreg`                                 | `ida_idp.ph_get_reg_last_sreg`                               |                                                              |
| `ida_idp.removing_func_tail`                                 | `ida_idp.deleting_func_tail`                                 |                                                              |
| `ida_idp.segm_attrs_changed`                                 | `ida_idp.segm_attrs_updated`                                 |                                                              |
| `ida_idp.str2regf`                                           | `ida_idp.str2reg`                                            |                                                              |
| `ida_idp.parse_reg_name(regname, reg_info_t)`                | `ida_idp.parse_reg_name(reg_info_t, regname)`                |                                                              |
| `ida_dbg.get_process_info`                                   | `ida_dbg.get_processes`                                      |                                                              |
| `ida_dbg.get_process_qty`                                    | `ida_dbg.get_processes`                                      |                                                              |
| `ida_funcs.FUNC_STATIC`                                      | `ida_funcs.FUNC_STATICDEF`                                   |                                                              |
| `ida_funcs.add_regarg2`                                      | `ida_funcs.add_regarg`                                       |                                                              |
| `ida_funcs.clear_func_struct`                                | `lambda *args: True`                                         |                                                              |
| `ida_funcs.del_func_cmt(pfn, rpt)`                           | `ida_funcs.set_func_cmt(pfn, "", rpt)`                       |                                                              |
| `ida_funcs.func_parent_iterator_set2`                        | `ida_funcs.func_parent_iterator_set`                         |                                                              |
| `ida_funcs.func_setend`                                      | `ida_funcs.set_func_end`                                     |                                                              |
| `ida_funcs.func_setstart`                                    | `ida_funcs.set_func_start`                                   |                                                              |
| `ida_funcs.func_tail_iterator_set2`                          | `ida_funcs.func_tail_iterator_set`                           |                                                              |
| `ida_funcs.get_func_limits(pfn, limits)`                     | `    import ida_range    rs = ida_range.rangeset_t()    if get_func_ranges(rs, pfn) == ida_idaapi.BADADDR:        return False    limits.start_ea = rs.begin().start_ea    limits.end_ea = rs.begin().end_ea ` |                                                              |
| `ida_funcs.get_func_name2`                                   | `ida_funcs.get_func_name`                                    |                                                              |
| `ida_name.demangle_name(name, mask)`                         | `ida_name.demangle_name(name, mask, ida_name.DQT_FULL)`      |                                                              |
| `ida_name.demangle_name2`                                    | `ida_name.demangle_name`                                     |                                                              |
| `ida_name.do_name_anyway(ea, name, maxlen)`                  | `ida_name.force_name(ea, name)`                              |                                                              |
| `ida_name.extract_name2`                                     | `ida_name.extract_name`                                      |                                                              |
| `ida_name.get_debug_name2`                                   | `ida_name.get_debug_name`                                    |                                                              |
| `ida_name.get_true_name`                                     | `ida_name.get_name`                                          |                                                              |
| `ida_name.is_ident_char`                                     | `ida_name.is_ident_cp`                                       |                                                              |
| `ida_name.is_visible_char`                                   | `ida_name.is_visible_cp`                                     |                                                              |
| `ida_name.make_visible_name(name, size)`                     | `ida_name.validate_name(name, ida_name.VNT_VISIBLE)`         |                                                              |
| `ida_name.validate_name2(name, size)`                        | `ida_name.validate_name(name, ida_name.VNT_IDENT)`           |                                                              |
| `ida_name.validate_name3(name)`                              | `ida_name.validate_name(name, ida_name.VNT_IDENT)`           |                                                              |
| `ida_name.isident`                                           | `ida_name.is_ident`                                          |                                                              |
| `ida_name.get_name(from, ea)`                                | `ida_name.get_name(ea)`                                      |                                                              |
| `ida_name.GN_INSNLOC`                                        | `0`                                                          |                                                              |
| `ida_enum.CONST_ERROR_ENUM`                                  | `ida_enum.ENUM_MEMBER_ERROR_NAME`                            |                                                              |
| `ida_enum.CONST_ERROR_ILLV`                                  | `ida_enum.ENUM_MEMBER_ERROR_VALUE`                           |                                                              |
| `ida_enum.CONST_ERROR_MASK`                                  | `ida_enum.ENUM_MEMBER_ERROR_ENUM`                            |                                                              |
| `ida_enum.CONST_ERROR_NAME`                                  | `ida_enum.ENUM_MEMBER_ERROR_MASK`                            |                                                              |
| `ida_enum.CONST_ERROR_VALUE`                                 | `ida_enum.ENUM_MEMBER_ERROR_ILLV`                            |                                                              |
| `ida_enum.add_const`                                         | `ida_enum.add_enum_member`                                   |                                                              |
| `ida_enum.del_const`                                         | `ida_enum.del_enum_member`                                   |                                                              |
| `ida_enum.get_const`                                         | `ida_enum.get_enum_member`                                   |                                                              |
| `ida_enum.get_const_bmask`                                   | `ida_enum.get_enum_member_bmask`                             |                                                              |
| `ida_enum.get_const_by_name`                                 | `ida_enum.get_enum_member_by_name`                           |                                                              |
| `ida_enum.get_const_cmt`                                     | `ida_enum.get_enum_member_cmt`                               |                                                              |
| `ida_enum.get_const_enum`                                    | `ida_enum.get_enum_member_enum`                              |                                                              |
| `ida_enum.get_const_name`                                    | `ida_enum.get_enum_member_name`                              |                                                              |
| `ida_enum.get_const_serial`                                  | `ida_enum.get_enum_member_serial`                            |                                                              |
| `ida_enum.get_const_value`                                   | `ida_enum.get_enum_member_value`                             |                                                              |
| `ida_enum.get_first_const`                                   | `ida_enum.get_first_enum_member`                             |                                                              |
| `ida_enum.get_first_serial_const`                            | `ida_enum.get_first_serial_enum_member`                      |                                                              |
| `ida_enum.get_last_const`                                    | `ida_enum.get_last_enum_member`                              |                                                              |
| `ida_enum.get_last_serial_const`                             | `ida_enum.get_last_serial_enum_member`                       |                                                              |
| `ida_enum.get_next_const`                                    | `ida_enum.get_next_enum_member`                              |                                                              |
| `ida_enum.get_next_serial_const`                             | `ida_enum.get_next_serial_enum_member`                       |                                                              |
| `ida_enum.get_prev_const`                                    | `ida_enum.get_prev_enum_member`                              |                                                              |
| `ida_enum.get_prev_serial_const`                             | `ida_enum.get_prev_serial_enum_member`                       |                                                              |
| `ida_enum.set_const_cmt`                                     | `ida_enum.set_enum_member_cmt`                               |                                                              |
| `ida_enum.set_const_name`                                    | `ida_enum.set_enum_member_name`                              |                                                              |
| `ida_enum.get_next_serial_enum_member(cid, serial)`          | `ida_enum.get_next_serial_enum_member(serial, cid)`          |                                                              |
| `ida_enum.get_prev_serial_enum_member(cid, serial)`          | `ida_enum.get_prev_serial_enum_member(serial, cid)`          |                                                              |
| `ida_expr.Compile`                                           | `ida_expr.compile_idc_file`                                  |                                                              |
| `ida_expr.CompileEx`                                         | `ida_expr.compile_idc_file`                                  |                                                              |
| `ida_expr.CompileLine`                                       | `ida_expr.compile_idc_text`                                  |                                                              |
| `ida_expr.VT_STR2`                                           | `ida_expr.VT_STR`                                            |                                                              |
| `ida_expr.VarCopy`                                           | `ida_expr.copy_idcv`                                         |                                                              |
| `ida_expr.VarDelAttr`                                        | `ida_expr.del_idcv_attr`                                     |                                                              |
| `ida_expr.VarDeref`                                          | `ida_expr.deref_idcv`                                        |                                                              |
| `ida_expr.VarFirstAttr`                                      | `ida_expr.first_idcv_attr`                                   |                                                              |
| `ida_expr.VarGetAttr(obj, attr, res, may_use_getattr=False)` | `ida_expr.get_idcv_attr(res, obj, attr, may_use_getattr)`    |                                                              |
| `ida_expr.VarGetClassName`                                   | `ida_expr.get_idcv_class_name`                               |                                                              |
| `ida_expr.VarGetSlice`                                       | `ida_expr.get_idcv_slice`                                    |                                                              |
| `ida_expr.VarInt64`                                          | `ida_expr.idcv_int64`                                        |                                                              |
| `ida_expr.VarLastAttr`                                       | `ida_expr.last_idcv_attr`                                    |                                                              |
| `ida_expr.VarMove`                                           | `ida_expr.move_idcv`                                         |                                                              |
| `ida_expr.VarNextAttr`                                       | `ida_expr.next_idcv_attr`                                    |                                                              |
| `ida_expr.VarObject`                                         | `ida_expr.idcv_object`                                       |                                                              |
| `ida_expr.VarPrevAttr`                                       | `ida_expr.prev_idcv_attr`                                    |                                                              |
| `ida_expr.VarPrint`                                          | `ida_expr.print_idcv`                                        |                                                              |
| `ida_expr.VarRef`                                            | `ida_expr.create_idcv_ref`                                   |                                                              |
| `ida_expr.VarSetAttr`                                        | `ida_expr.set_idcv_attr`                                     |                                                              |
| `ida_expr.VarSetSlice`                                       | `ida_expr.set_idcv_slice`                                    |                                                              |
| `ida_expr.VarString2`                                        | `ida_expr.idcv_string`                                       |                                                              |
| `ida_expr.VarSwap`                                           | `ida_expr.swap_idcvs`                                        |                                                              |
| `ida_expr.calc_idc_expr(where, expr, res)`                   | `ida_expr.eval_idc_expr(res, where, expr)`                   |                                                              |
| `ida_expr.calcexpr(where, expr, res)`                        | `ida_expr.eval_expr(res, where, expr)`                       |                                                              |
| `ida_expr.dosysfile(complain_if_no_file, fname)`             | `ida_expr.exec_system_script(fname, complain_if_no_file)`    |                                                              |
| `ida_expr.execute(line)`                                     | `ida_expr.eval_idc_snippet(None, line)`                      |                                                              |
| `ida_expr.py_set_idc_func_ex`                                | `ida_expr.py_add_idc_func`                                   |                                                              |
| `ida_expr.set_idc_func_ex(name, fp=None, args=(), flags=0)`  | `ida_expr.add_idc_func(name, fp, args, (), flags)`           |                                                              |
| `ida_auto.analyze_area`                                      | `ida_auto.plan_and_wait`                                     |                                                              |
| `ida_auto.autoCancel`                                        | `ida_auto.auto_cancel`                                       |                                                              |
| `ida_auto.autoIsOk`                                          | `ida_auto.auto_is_ok`                                        |                                                              |
| `ida_auto.autoMark`                                          | `ida_auto.auto_mark`                                         |                                                              |
| `ida_auto.autoUnmark`                                        | `ida_auto.auto_unmark`                                       |                                                              |
| `ida_auto.autoWait`                                          | `ida_auto.auto_wait`                                         |                                                              |
| `ida_auto.noUsed`                                            | `ida_auto.plan_ea`                                           |                                                              |
| `ida_auto.setStat`                                           | `ida_auto.set_ida_state`                                     |                                                              |
| `ida_auto.showAddr`                                          | `ida_auto.show_addr`                                         |                                                              |
| `ida_auto.showAuto`                                          | `ida_auto.show_auto`                                         |                                                              |
| `ida_nalt.ASCSTR_LAST`                                       | `7`                                                          |                                                              |
| `ida_nalt.ASCSTR_LEN2`                                       | `ida_nalt.STRTYPE_LEN2`                                      |                                                              |
| `ida_nalt.ASCSTR_LEN4`                                       | `ida_nalt.STRTYPE_LEN4`                                      |                                                              |
| `ida_nalt.ASCSTR_PASCAL`                                     | `ida_nalt.STRTYPE_PASCAL`                                    |                                                              |
| `ida_nalt.ASCSTR_TERMCHR`                                    | `ida_nalt.STRTYPE_TERMCHR`                                   |                                                              |
| `ida_nalt.ASCSTR_ULEN2`                                      | `ida_nalt.STRTYPE_LEN2_16`                                   |                                                              |
| `ida_nalt.ASCSTR_ULEN4`                                      | `ida_nalt.STRTYPE_LEN4_16`                                   |                                                              |
| `ida_nalt.ASCSTR_UNICODE`                                    | `ida_nalt.STRTYPE_C_16`                                      |                                                              |
| `ida_nalt.ASCSTR_UTF16`                                      | `ida_nalt.STRTYPE_C_16`                                      |                                                              |
| `ida_nalt.ASCSTR_UTF32`                                      | `ida_nalt.STRTYPE_C_32`                                      |                                                              |
| `ida_nalt.REF_VHIGH`                                         | `ida_nalt.V695_REF_VHIGH`                                    |                                                              |
| `ida_nalt.REF_VLOW`                                          | `ida_nalt.V695_REF_VLOW`                                     |                                                              |
| `ida_nalt.SWI_END_IN_TBL`                                    | `ida_nalt.SWI_DEF_IN_TBL`                                    |                                                              |
| `ida_nalt.SWI_BC695_EXTENDED`                                | `0x8000`                                                     |                                                              |
| `ida_nalt.SWI2_INDIRECT`                                     | `ida_nalt.SWI_INDIRECT >> 16`                                |                                                              |
| `ida_nalt.SWI2_SUBTRACT`                                     | `ida_nalt.SWI_SUBTRACT >> 16`                                |                                                              |
| `ida_nalt.RIDX_AUTO_PLUGINS`                                 | `ida_netnode.BADNODE`                                        |                                                              |
| `ida_nalt.change_encoding_name`                              | `ida_nalt.rename_encoding`                                   |                                                              |
| `ida_nalt.del_tinfo2(ea, n=None)`                            | `ida_nalt.del_op_tinfo(ea, n) if n is not None else ida_nalt.del_tinfo(ea)` |                                                              |
| `ida_nalt.get_encodings_count`                               | `ida_nalt.get_encoding_qty`                                  |                                                              |
| `ida_nalt.get_op_tinfo(ea, n, tinfo_t)`                      | `ida_nalt.get_op_tinfo(tinfo_t, ea, n)`                      |                                                              |
| `ida_nalt.get_op_tinfo2`                                     | `ida_nalt.get_op_tinfo`                                      |                                                              |
| `ida_nalt.is_unicode(strtype)`                               | `(strtype & STRWIDTH_MASK) > 0`                              |                                                              |
| `ida_nalt.set_op_tinfo2`                                     | `ida_nalt.set_op_tinfo`                                      |                                                              |
| `ida_nalt.set_tinfo2`                                        | `ida_nalt.set_tinfo`                                         |                                                              |
| `ida_nalt.switch_info_t.regdtyp`                             | `ida_nalt.switch_info_t.regdtype`                            |                                                              |
| `ida_nalt.get_tinfo(ea, tinfo_t)`                            | `ida_nalt.get_tinfo(tinfo_t, ea)`                            |                                                              |
| `ida_nalt.get_tinfo2`                                        | `ida_nalt.get_tinfo`                                         |                                                              |
| `ida_nalt.get_refinfo(ea, n, refinfo)`                       | `ida_nalt.get_refinfo(refinfo, ea, n)`                       |                                                              |
| `ida_nalt.get_switch_info_ex`                                | `ida_nalt.get_switch_info`                                   |                                                              |
| `ida_nalt.set_switch_info_ex`                                | `ida_nalt.set_switch_info`                                   |                                                              |
| `ida_nalt.del_switch_info_ex`                                | `ida_nalt.del_switch_info`                                   |                                                              |
| `ida_nalt.switch_info_t.flags`                               | `ida_nalt.switch_info_t.flags`                               | Flags have been modified a bit. Please see nalt.hpp for more info |
| `ida_nalt.switch_info_t.flags2`                              | `ida_nalt.switch_info_t.flags`                               | Flags have been modified a bit. Please see nalt.hpp for more info |
| `ida_nalt.switch_info_ex_t`                                  | `ida_nalt.switch_info_t`                                     |                                                              |
| `ida_graph.clr_node_info2`                                   | `ida_graph.clr_node_info`                                    |                                                              |
| `ida_graph.del_node_info2`                                   | `ida_graph.del_node_info`                                    |                                                              |
| `ida_graph.get_node_info2`                                   | `ida_graph.get_node_info`                                    |                                                              |
| `ida_graph.set_node_info2`                                   | `ida_graph.set_node_info`                                    |                                                              |
| `ida_graph.GraphViewer.GetTForm`                             | `ida_graph.GraphViewer.GetWidget`                            |                                                              |
| `ida_typeinf.BFI_NOCONST`                                    | `0`                                                          |                                                              |
| `ida_typeinf.BFI_NOLOCS`                                     | `0`                                                          |                                                              |
| `ida_typeinf.NTF_NOIDB`                                      | `0`                                                          |                                                              |
| `ida_typeinf.PRVLOC_STKOFF`                                  | `ida_typeinf.PRALOC_VERIFY`                                  |                                                              |
| `ida_typeinf.PRVLOC_VERIFY`                                  | `ida_typeinf.PRALOC_STKOFF`                                  |                                                              |
| `ida_typeinf.TERR_TOOLONGNAME`                               | `ida_typeinf.TERR_WRONGNAME`                                 |                                                              |
| `ida_typeinf.add_til(name)`                                  | `ida_typeinf.add_til(name, flags)`                           |                                                              |
| `ida_typeinf.add_til2`                                       | `ida_typeinf.add_til`                                        |                                                              |
| `ida_typeinf.apply_decl`                                     | `ida_typeinf.apply_cdecl`                                    |                                                              |
| `ida_typeinf.apply_cdecl2`                                   | `ida_typeinf.apply_cdecl`                                    |                                                              |
| `ida_typeinf.apply_tinfo2`                                   | `ida_typeinf.apply_tinfo`                                    |                                                              |
| `ida_typeinf.calc_c_cpp_name4`                               | `ida_typeinf.calc_c_cpp_name`                                |                                                              |
| `ida_typeinf.choose_local_type`                              | `ida_typeinf.choose_local_tinfo`                             |                                                              |
| `ida_typeinf.choose_named_type2`                             | `ida_typeinf.choose_named_type`                              |                                                              |
| `ida_typeinf.deref_ptr2`                                     | `ida_typeinf.deref_ptr`                                      |                                                              |
| `ida_typeinf.extract_varloc`                                 | `ida_typeinf.extract_argloc`                                 |                                                              |
| `ida_typeinf.const_vloc_visitor_t`                           | `ida_typeinf.const_aloc_visitor_t`                           |                                                              |
| `ida_typeinf.for_all_const_varlocs`                          | `ida_typeinf.for_all_const_arglocs`                          |                                                              |
| `ida_typeinf.for_all_varlocs`                                | `ida_typeinf.for_all_arglocs`                                |                                                              |
| `ida_typeinf.gen_decorate_name3(name, mangle, cc)`           | `ida_typeinf.gen_decorate_name(name, mangle, cc, None)`      |                                                              |
| `ida_typeinf.get_enum_member_expr2`                          | `ida_typeinf.get_enum_member_expr`                           |                                                              |
| `ida_typeinf.get_idainfo_by_type3`                           | `ida_typeinf.get_idainfo_by_type`                            |                                                              |
| `ida_typeinf.guess_func_tinfo2(pfn, tif)`                    | `ida_typeinf.guess_tinfo(pfn.start_ea, tif)`                 |                                                              |
| `ida_typeinf.load_til2`                                      | `ida_typeinf.load_til`                                       |                                                              |
| `ida_typeinf.lower_type2`                                    | `ida_typeinf.lower_type`                                     |                                                              |
| `ida_typeinf.optimize_varloc`                                | `ida_typeinf.optimize_argloc`                                |                                                              |
| `ida_typeinf.parse_decl2(til, decl, tif, flags)`             | `ida_typeinf.parse_decl(tif, til, decl, flags)`              |                                                              |
| `ida_typeinf.print_type(ea, )`                               | `ida_typeinf.print_type(ea, PRTYPE_1LINE if else 0)`         |                                                              |
| `ida_typeinf.print_type2`                                    | `ida_typeinf.print_type`                                     |                                                              |
| `ida_typeinf.print_type3`                                    | `ida_typeinf.print_type`                                     |                                                              |
| `ida_typeinf.print_varloc`                                   | `ida_typeinf.print_argloc`                                   |                                                              |
| `ida_typeinf.resolve_typedef2`                               | `ida_typeinf.resolve_typedef`                                |                                                              |
| `ida_typeinf.scattered_vloc_t`                               | `ida_typeinf.scattered_aloc_t`                               |                                                              |
| `ida_typeinf.set_compiler2`                                  | `ida_typeinf.set_compiler`                                   |                                                              |
| `ida_typeinf.varloc_t`                                       | `ida_typeinf.argloc_t`                                       |                                                              |
| `ida_typeinf.varpart_t`                                      | `ida_typeinf.argpart_t`                                      |                                                              |
| `ida_typeinf.verify_varloc`                                  | `ida_typeinf.verify_argloc`                                  |                                                              |
| `ida_typeinf.vloc_visitor_t`                                 | `ida_typeinf.aloc_visitor_t`                                 |                                                              |
| `ida_typeinf.guess_tinfo(id, tinfo_t)`                       | `ida_typeinf.guess_tinfo(tinfo_t, id)`                       |                                                              |
| `ida_typeinf.guess_tinfo2`                                   | `ida_typeinf.guess_tinfo`                                    |                                                              |
| `ida_typeinf.find_tinfo_udt_member(typid, strmem_flags, udm)` | `ida_typeinf.find_tinfo_udt_member(udm, typid, strmem_flags)` |                                                              |
| `ida_typeinf.find_udt_member(strmem_flags, udm)`             | `ida_typeinf.find_udt_member(udm, strmem_flags)`             |                                                              |
| `ida_typeinf.save_tinfo(til_t, size_t, name, int, tinfo_t)`  | `ida_typeinf.save_tinfo(tinfo_t, til_t, size_t, name, int)`  |                                                              |
| `ida_ua.codeSeg(ea, opnum)`                                  | ` insn = ida_ua.insn_t() if ida_ua.decode_insn(insn, ea):     x = ida_ua.map_code_ea(insn, insn.ops[opnum]) else:     x = ida_idaapi.BADADDR ` |                                                              |
| `ida_ua.get_dtyp_by_size`                                    | `ida_ua.get_dtype_by_size`                                   |                                                              |
| `ida_ua.get_dtyp_flag`                                       | `ida_ua.get_dtype_flag`                                      |                                                              |
| `ida_ua.get_dtyp_size`                                       | `ida_ua.get_dtype_size`                                      |                                                              |
| `ida_ua.get_operand_immvals`                                 | `ida_ua.get_immvals`                                         |                                                              |
| `ida_ua.op_t.dtyp`                                           | `ida_ua.op_t.dtype`                                          |                                                              |
| `ida_ua.cmd`                                                 | `ida_ua.insn_t()`                                            | 'cmd' doesn't exist anymore                                  |
| `ida_ua.decode_insn(ea)`                                     | `ida_ua.decode_insn(insn_t, ea)`                             |                                                              |
| `ida_ua.create_insn(ea)`                                     | `ida_ua.create_insn(insn_t, ea)`                             |                                                              |
| `ida_ua.decode_prev_insn(ea)`                                | `ida_ua.decode_prev_insn(insn_t, ea)`                        |                                                              |
| `ida_ua.decode_preceding_insn(ea)`                           | `ida_ua.decode_preceding_insn(insn_t, ea)`                   |                                                              |
| `ida_ua.UA_MAXOP`                                            | `ida_ida.UA_MAXOP`                                           |                                                              |
| `ida_ua.dt_3byte`                                            | `ida_ua.dt_byte`                                             |                                                              |
| `ida_ua.tbo_123`                                             | `0`                                                          |                                                              |
| `ida_ua.tbo_132`                                             | `0`                                                          |                                                              |
| `ida_ua.tbo_213`                                             | `0`                                                          |                                                              |
| `ida_ua.tbo_231`                                             | `0`                                                          |                                                              |
| `ida_ua.tbo_312`                                             | `0`                                                          |                                                              |
| `ida_ua.tbo_321`                                             | `0`                                                          |                                                              |
| `ida_ua.ua_add_cref(opoff, to, rtype)`                       | `ida_ua.insn_t.add_cref(to, opoff, rtype)`                   |                                                              |
| `ida_ua.ua_add_dref(opoff, to, rtype)`                       | `ida_ua.insn_t.add_dref(to, opoff, rtype)`                   |                                                              |
| `ida_ua.ua_add_off_drefs(x, rtype)`                          | `ida_ua.insn_t.add_off_drefs(x, rtype, 0)`                   |                                                              |
| `ida_ua.ua_add_off_drefs2(x, rtype, outf)`                   | `ida_ua.insn_t.add_off_drefs(x, rtype, outf)`                |                                                              |
| `ida_ua.ua_dodata(ea, dtype)`                                | `ida_ua.insn_t.create_op_data(ea, 0, dtype)`                 |                                                              |
| `ida_ua.ua_dodata2(opoff, ea, dtype)`                        | `ida_ua.insn_t.create_op_data(ea, opoff, dtype)`             |                                                              |
| `ida_ua.ua_stkvar2(x, v, flags)`                             | `ida_ua.insn_t.create_stkvar(x, v, flags)`                   |                                                              |
| `ida_diskio.create_generic_linput64`                         | `ida_diskio.create_generic_linput`                           |                                                              |
| `ida_diskio.generic_linput64_t`                              | `ida_diskio.generic_linput_t`                                |                                                              |
| `ida_offset.calc_reference_basevalue`                        | `ida_offset.calc_basevalue`                                  |                                                              |
| `ida_offset.calc_reference_target`                           | `ida_offset.calc_target`                                     |                                                              |
| `ida_offset.set_offset(ea, n, base)`                         | `ida_offset.op_offset(ea, n, ida_ua.get_default_reftype(ea), ida_idaapi.BADADDR, base) > 0` |                                                              |
| `ida_netnode.netnode.alt1st`                                 | `ida_netnode.netnode.altfirst`                               |                                                              |
| `ida_netnode.netnode.alt1st_idx8`                            | `ida_netnode.netnode.altfirst_idx8`                          |                                                              |
| `ida_netnode.netnode.altnxt`                                 | `ida_netnode.netnode.altnext`                                |                                                              |
| `ida_netnode.netnode.char1st`                                | `ida_netnode.netnode.charfirst`                              |                                                              |
| `ida_netnode.netnode.char1st_idx8`                           | `ida_netnode.netnode.charfirst_idx8`                         |                                                              |
| `ida_netnode.netnode.charnxt`                                | `ida_netnode.netnode.charnext`                               |                                                              |
| `ida_netnode.netnode.hash1st`                                | `ida_netnode.netnode.hashfirst`                              |                                                              |
| `ida_netnode.netnode.hashnxt`                                | `ida_netnode.netnode.hashnext`                               |                                                              |
| `ida_netnode.netnode.sup1st`                                 | `ida_netnode.netnode.supfirst`                               |                                                              |
| `ida_netnode.netnode.sup1st_idx8`                            | `ida_netnode.netnode.supfirst_idx8`                          |                                                              |
| `ida_netnode.netnode.supnxt`                                 | `ida_netnode.netnode.supnext`                                |                                                              |
| `ida_struct.get_member_name2`                                | `ida_struct.get_member_name`                                 |                                                              |
| `ida_struct.get_member_tinfo(mptr, tinfo_t)`                 | `ida_struct.get_member_tinfo(tinfo_t, mptr)`                 |                                                              |
| `ida_struct.get_or_guess_member_tinfo(mptr, tinfo_t)`        | `ida_struct.get_or_guess_member_tinfo(tinfo_t, mptr)`        |                                                              |
| `ida_struct.get_member_tinfo2`                               | `ida_struct.get_member_tinfo`                                |                                                              |
| `ida_struct.get_or_guess_member_tinfo2`                      | `ida_struct.get_or_guess_member_tinfo`                       |                                                              |
| `ida_struct.save_struc2`                                     | `ida_struct.save_struc`                                      |                                                              |
| `ida_struct.set_member_tinfo2`                               | `ida_struct.set_member_tinfo`                                |                                                              |
| `ida_ida.AF2_ANORET`                                         | `ida_ida.AF_ANORET`                                          |                                                              |
| `ida_ida.AF2_CHKUNI`                                         | `ida_ida.AF_CHKUNI`                                          |                                                              |
| `ida_ida.AF2_DATOFF`                                         | `ida_ida.AF_DATOFF`                                          |                                                              |
| `ida_ida.AF2_DOCODE`                                         | `ida_ida.AF_DOCODE`                                          |                                                              |
| `ida_ida.AF2_DODATA`                                         | `ida_ida.AF_DODATA`                                          |                                                              |
| `ida_ida.AF2_FTAIL`                                          | `ida_ida.AF_FTAIL`                                           |                                                              |
| `ida_ida.AF2_HFLIRT`                                         | `ida_ida.AF_HFLIRT`                                          |                                                              |
| `ida_ida.AF2_JUMPTBL`                                        | `ida_ida.AF_JUMPTBL`                                         |                                                              |
| `ida_ida.AF2_MEMFUNC`                                        | `ida_ida.AF_MEMFUNC`                                         |                                                              |
| `ida_ida.AF2_PURDAT`                                         | `ida_ida.AF_PURDAT`                                          |                                                              |
| `ida_ida.AF2_REGARG`                                         | `ida_ida.AF_REGARG`                                          |                                                              |
| `ida_ida.AF2_SIGCMT`                                         | `ida_ida.AF_SIGCMT`                                          |                                                              |
| `ida_ida.AF2_SIGMLT`                                         | `ida_ida.AF_SIGMLT`                                          |                                                              |
| `ida_ida.AF2_STKARG`                                         | `ida_ida.AF_STKARG`                                          |                                                              |
| `ida_ida.AF2_TRFUNC`                                         | `ida_ida.AF_TRFUNC`                                          |                                                              |
| `ida_ida.AF2_VERSP`                                          | `ida_ida.AF_VERSP`                                           |                                                              |
| `ida_ida.AF_ASCII`                                           | `ida_ida.AF_STRLIT`                                          |                                                              |
| `ida_ida.ASCF_AUTO`                                          | `ida_ida.STRF_AUTO`                                          |                                                              |
| `ida_ida.ASCF_COMMENT`                                       | `ida_ida.STRF_COMMENT`                                       |                                                              |
| `ida_ida.ASCF_GEN`                                           | `ida_ida.STRF_GEN`                                           |                                                              |
| `ida_ida.ASCF_SAVECASE`                                      | `ida_ida.STRF_SAVECASE`                                      |                                                              |
| `ida_ida.ASCF_SERIAL`                                        | `ida_ida.STRF_SERIAL`                                        |                                                              |
| `ida_ida.ASCF_UNICODE`                                       | `ida_ida.STRF_UNICODE`                                       |                                                              |
| `ida_ida.INFFL_LZERO`                                        | `ida_ida.OFLG_LZERO`                                         |                                                              |
| `ida_ida.ansi2idb`                                           | `ida_ida.lambda thing: thing`                                |                                                              |
| `ida_ida.idb2scr`                                            | `ida_ida.lambda thing: thing`                                |                                                              |
| `ida_ida.scr2idb`                                            | `ida_ida.lambda thing: thing`                                |                                                              |
| `ida_ida.showAllComments`                                    | `ida_ida.show_all_comments`                                  |                                                              |
| `ida_ida.showComments`                                       | `ida_ida.show_comments`                                      |                                                              |
| `ida_ida.showRepeatables`                                    | `ida_ida.show_repeatables`                                   |                                                              |
| `ida_ida.toEA`                                               | `ida_ida.to_ea`                                              |                                                              |
| `ida_ida.idainfo.ASCIIbreak`                                 | `ida_ida.idainfo.strlit_break`                               |                                                              |
| `ida_ida.idainfo.ASCIIpref`                                  | `ida_ida.idainfo.strlit_pref`                                |                                                              |
| `ida_ida.idainfo.ASCIIsernum`                                | `ida_ida.idainfo.strlit_sernum`                              |                                                              |
| `ida_ida.idainfo.ASCIIzeroes`                                | `ida_ida.idainfo.strlit_zeroes`                              |                                                              |
| `ida_ida.idainfo.asciiflags`                                 | `ida_ida.idainfo.strlit_flags`                               |                                                              |
| `ida_ida.idainfo.beginEA`                                    | `ida_ida.idainfo.start_ea`                                   |                                                              |
| `ida_ida.idainfo.binSize`                                    | `ida_ida.idainfo.bin_prefix_size`                            |                                                              |
| `ida_ida.idainfo.get_proc_name`                              | `[ida_ida.idainfo.procname, ida_ida.idainfo.procname]`       |                                                              |
| `ida_ida.idainfo.graph_view`                                 | `ida_ida.idainfo.is_graph_view and ida_ida.idainfo.set_graph_view` |                                                              |
| `ida_ida.idainfo.mf`                                         | `ida_ida.idainfo.is_be and ida_ida.idainfo.set_be`           |                                                              |
| `ida_ida.idainfo.namelen`                                    | `ida_ida.idainfo.max_autoname_len`                           |                                                              |
| `ida_ida.idainfo.omaxEA`                                     | `ida_ida.idainfo.omax_ea`                                    |                                                              |
| `ida_ida.idainfo.ominEA`                                     | `ida_ida.idainfo.omin_ea`                                    |                                                              |
| `ida_ida.idainfo.s_assume`                                   | `ida_ida.idainfo.outflags binary operations with: OFLG_GEN_ASSUME` |                                                              |
| `ida_ida.idainfo.s_auto`                                     | `ida_ida.idainfo.is_auto_enabled and ida_ida.idainfo.set_auto_enabled` |                                                              |
| `ida_ida.idainfo.s_null`                                     | `ida_ida.idainfo.outflags binary operations with: OFLG_GEN_NULL` |                                                              |
| `ida_ida.idainfo.s_org`                                      | `ida_ida.idainfo.outflags binary operations with: OFLG_GEN_ORG` |                                                              |
| `ida_ida.idainfo.s_prefseg`                                  | `ida_ida.idainfo.outflags binary operations with: OFLG_PREF_SEG` |                                                              |
| `ida_ida.idainfo.s_showauto`                                 | `ida_ida.idainfo.outflags binary operations with: OFLG_SHOW_AUTO` |                                                              |
| `ida_ida.idainfo.s_showpref`                                 | `ida_ida.idainfo.outflags binary operations with: OFLG_SHOW_PREF` |                                                              |
| `ida_ida.idainfo.s_void`                                     | `ida_ida.idainfo.outflags binary operations with: OFLG_SHOW_VOID` |                                                              |
| `ida_ida.idainfo.startIP`                                    | `ida_ida.idainfo.start_ip`                                   |                                                              |
| `ida_ida.idainfo.startSP`                                    | `ida_ida.idainfo.start_sp`                                   |                                                              |
| `ida_ida.idainfo.wide_high_byte_first`                       | `ida_ida.idainfo.lflags binary operations with: LFLG_WIDE_HBF` |                                                              |
| `ida_ida.idainfo.allow_nonmatched_ops`                       | ``                                                           | Gone entirely                                                |
| `ida_ida.idainfo.check_manual_ops`                           | ``                                                           | Gone entirely                                                |
| `ida_fixup.FIXUP_CREATED`                                    | `ida_fixup.FIXUPF_CREATED`                                   |                                                              |
| `ida_fixup.FIXUP_EXTDEF`                                     | `ida_fixup.FIXUPF_EXTDEF`                                    |                                                              |
| `ida_fixup.FIXUP_REL`                                        | `ida_fixup.FIXUPF_REL`                                       |                                                              |
| `ida_bytes.ACFOPT_ASCII`                                     | `0`                                                          |                                                              |
| `ida_bytes.ACFOPT_CONVMASK`                                  | `0`                                                          |                                                              |
| `ida_bytes.ACFOPT_ESCAPE`                                    | `ida_bytes.STRCONV_ESCAPE`                                   |                                                              |
| `ida_bytes.ACFOPT_UTF16`                                     | `0`                                                          |                                                              |
| `ida_bytes.ACFOPT_UTF8`                                      | `0`                                                          |                                                              |
| `ida_bytes.DOUNK_DELNAMES`                                   | `ida_bytes.DELIT_DELNAMES`                                   |                                                              |
| `ida_bytes.DOUNK_EXPAND`                                     | `ida_bytes.DELIT_EXPAND`                                     |                                                              |
| `ida_bytes.DOUNK_NOTRUNC`                                    | `ida_bytes.DELIT_NOTRUNC`                                    |                                                              |
| `ida_bytes.DOUNK_SIMPLE`                                     | `ida_bytes.DELIT_SIMPLE`                                     |                                                              |
| `ida_bytes.FF_ASCI`                                          | `ida_bytes.FF_STRLIT`                                        |                                                              |
| `ida_bytes.FF_DWRD`                                          | `ida_bytes.FF_DWORD`                                         |                                                              |
| `ida_bytes.FF_OWRD`                                          | `ida_bytes.FF_OWORD`                                         |                                                              |
| `ida_bytes.FF_QWRD`                                          | `ida_bytes.FF_QWORD`                                         |                                                              |
| `ida_bytes.FF_STRU`                                          | `ida_bytes.FF_STRUCT`                                        |                                                              |
| `ida_bytes.FF_TBYT`                                          | `ida_bytes.FF_TBYTE`                                         |                                                              |
| `ida_bytes.FF_VAR`                                           | `0`                                                          |                                                              |
| `ida_bytes.FF_YWRD`                                          | `ida_bytes.FF_YWORD`                                         |                                                              |
| `ida_bytes.FF_ZWRD`                                          | `ida_bytes.FF_ZWORD`                                         |                                                              |
| `ida_bytes.GFE_NOVALUE`                                      | `0`                                                          |                                                              |
| `ida_bytes.add_hidden_area`                                  | `ida_bytes.add_hidden_range`                                 |                                                              |
| `ida_bytes.asciflag`                                         | `ida_bytes.strlit_flag`                                      |                                                              |
| `ida_bytes.delValue`                                         | `ida_bytes.del_value`                                        |                                                              |
| `ida_bytes.del_hidden_area`                                  | `ida_bytes.del_hidden_range`                                 |                                                              |
| `ida_bytes.do16bit`                                          | `ida_bytes.create_16bit_data`                                |                                                              |
| `ida_bytes.do32bit`                                          | `ida_bytes.create_32bit_data`                                |                                                              |
| `ida_bytes.doAlign`                                          | `ida_bytes.create_align`                                     |                                                              |
| `ida_bytes.doByte`                                           | `ida_bytes.create_byte`                                      |                                                              |
| `ida_bytes.doCustomData`                                     | `ida_bytes.create_custdata`                                  |                                                              |
| `ida_bytes.doDouble`                                         | `ida_bytes.create_double`                                    |                                                              |
| `ida_bytes.doDwrd`                                           | `ida_bytes.create_dword`                                     |                                                              |
| `ida_bytes.doExtra`                                          | `ida_bytes.ida_idaapi._BC695.false_p`                        |                                                              |
| `ida_bytes.doFloat`                                          | `ida_bytes.create_float`                                     |                                                              |
| `ida_bytes.doImmd`                                           | `ida_bytes.set_immd`                                         |                                                              |
| `ida_bytes.doOwrd`                                           | `ida_bytes.create_oword`                                     |                                                              |
| `ida_bytes.doPackReal`                                       | `ida_bytes.create_packed_real`                               |                                                              |
| `ida_bytes.doQwrd`                                           | `ida_bytes.create_qword`                                     |                                                              |
| `ida_bytes.doStruct`                                         | `ida_bytes.create_struct`                                    |                                                              |
| `ida_bytes.doTbyt`                                           | `ida_bytes.create_tbyte`                                     |                                                              |
| `ida_bytes.doWord`                                           | `ida_bytes.create_word`                                      |                                                              |
| `ida_bytes.doYwrd`                                           | `ida_bytes.create_yword`                                     |                                                              |
| `ida_bytes.doZwrd`                                           | `ida_bytes.create_zword`                                     |                                                              |
| `ida_bytes.do_data_ex`                                       | `ida_bytes.create_data`                                      |                                                              |
| `ida_bytes.do_unknown`                                       | `ida_bytes.del_items`                                        |                                                              |
| `ida_bytes.do_unknown_range(ea, size, flags)`                | `ida_bytes.del_items(ea, flags, size)`                       |                                                              |
| `ida_bytes.dwrdflag`                                         | `ida_bytes.dword_flag`                                       |                                                              |
| `ida_bytes.f_hasRef`                                         | `ida_bytes.f_has_xref`                                       |                                                              |
| `ida_bytes.f_isASCII`                                        | `ida_bytes.f_is_strlit`                                      |                                                              |
| `ida_bytes.f_isAlign`                                        | `ida_bytes.f_is_align`                                       |                                                              |
| `ida_bytes.f_isByte`                                         | `ida_bytes.f_is_byte`                                        |                                                              |
| `ida_bytes.f_isCode`                                         | `ida_bytes.f_is_code`                                        |                                                              |
| `ida_bytes.f_isCustom`                                       | `ida_bytes.f_is_custom`                                      |                                                              |
| `ida_bytes.f_isData`                                         | `ida_bytes.f_is_data`                                        |                                                              |
| `ida_bytes.f_isDouble`                                       | `ida_bytes.f_is_double`                                      |                                                              |
| `ida_bytes.f_isDwrd`                                         | `ida_bytes.f_is_dword`                                       |                                                              |
| `ida_bytes.f_isFloat`                                        | `ida_bytes.f_is_float`                                       |                                                              |
| `ida_bytes.f_isHead`                                         | `ida_bytes.f_is_head`                                        |                                                              |
| `ida_bytes.f_isNotTail`                                      | `ida_bytes.f_is_not_tail`                                    |                                                              |
| `ida_bytes.f_isOwrd`                                         | `ida_bytes.f_is_oword`                                       |                                                              |
| `ida_bytes.f_isPackReal`                                     | `ida_bytes.f_is_pack_real`                                   |                                                              |
| `ida_bytes.f_isQwrd`                                         | `ida_bytes.f_is_qword`                                       |                                                              |
| `ida_bytes.f_isStruct`                                       | `ida_bytes.f_is_struct`                                      |                                                              |
| `ida_bytes.f_isTail`                                         | `ida_bytes.f_is_tail`                                        |                                                              |
| `ida_bytes.f_isTbyt`                                         | `ida_bytes.f_is_tbyte`                                       |                                                              |
| `ida_bytes.f_isWord`                                         | `ida_bytes.f_is_word`                                        |                                                              |
| `ida_bytes.f_isYwrd`                                         | `ida_bytes.f_is_yword`                                       |                                                              |
| `ida_bytes.getDefaultRadix`                                  | `ida_bytes.get_default_radix`                                |                                                              |
| `ida_bytes.getFlags`                                         | `ida_bytes.get_full_flags`                                   |                                                              |
| `ida_bytes.get_long`                                         | `ida_bytes.get_dword`                                        |                                                              |
| `ida_bytes.get_full_byte`                                    | `ida_bytes.get_wide_byte`                                    |                                                              |
| `ida_bytes.get_full_word`                                    | `ida_bytes.get_wide_word`                                    |                                                              |
| `ida_bytes.get_full_long`                                    | `ida_bytes.get_wide_dword`                                   |                                                              |
| `ida_bytes.get_original_long`                                | `ida_bytes.get_original_dword`                               |                                                              |
| `ida_bytes.put_long`                                         | `ida_bytes.put_dword`                                        |                                                              |
| `ida_bytes.patch_long`                                       | `ida_bytes.patch_dword`                                      |                                                              |
| `ida_bytes.add_long`                                         | `ida_bytes.add_dword`                                        |                                                              |
| `ida_bytes.getRadix`                                         | `ida_bytes.get_radix`                                        |                                                              |
| `ida_bytes.get_ascii_contents`                               | `ida_bytes.get_strlit_contents`                              |                                                              |
| `ida_bytes.get_ascii_contents2`                              | `ida_bytes.get_strlit_contents`                              |                                                              |
| `ida_bytes.get_flags_novalue`                                | `ida_bytes.get_flags`                                        |                                                              |
| `ida_bytes.get_hidden_area`                                  | `ida_bytes.get_hidden_range`                                 |                                                              |
| `ida_bytes.get_hidden_area_num`                              | `ida_bytes.get_hidden_range_num`                             |                                                              |
| `ida_bytes.get_hidden_area_qty`                              | `ida_bytes.get_hidden_range_qty`                             |                                                              |
| `ida_bytes.get_many_bytes`                                   | `ida_bytes.get_bytes`                                        |                                                              |
| `ida_bytes.get_many_bytes_ex`                                | `ida_bytes.get_bytes_and_mask`                               |                                                              |
| `ida_bytes.get_max_ascii_length`                             | `ida_bytes.get_max_strlit_length`                            |                                                              |
| `ida_bytes.get_next_hidden_area`                             | `ida_bytes.get_next_hidden_range`                            |                                                              |
| `ida_bytes.get_prev_hidden_area`                             | `ida_bytes.get_prev_hidden_range`                            |                                                              |
| `ida_bytes.get_zero_areas`                                   | `ida_bytes.get_zero_ranges`                                  |                                                              |
| `ida_bytes.getn_hidden_area`                                 | `ida_bytes.getn_hidden_range`                                |                                                              |
| `ida_bytes.hasExtra`                                         | `ida_bytes.has_extra_cmts`                                   |                                                              |
| `ida_bytes.hasRef`                                           | `ida_bytes.has_xref`                                         |                                                              |
| `ida_bytes.hasValue`                                         | `ida_bytes.has_value`                                        |                                                              |
| `ida_bytes.hidden_area_t`                                    | `ida_bytes.hidden_range_t`                                   |                                                              |
| `ida_bytes.isASCII`                                          | `ida_bytes.is_strlit`                                        |                                                              |
| `ida_bytes.isAlign`                                          | `ida_bytes.is_align`                                         |                                                              |
| `ida_bytes.isByte`                                           | `ida_bytes.is_byte`                                          |                                                              |
| `ida_bytes.isChar`                                           | `ida_bytes.is_char`                                          |                                                              |
| `ida_bytes.isChar0`                                          | `ida_bytes.is_char0`                                         |                                                              |
| `ida_bytes.isChar1`                                          | `ida_bytes.is_char1`                                         |                                                              |
| `ida_bytes.isCode`                                           | `ida_bytes.is_code`                                          |                                                              |
| `ida_bytes.isCustFmt`                                        | `ida_bytes.is_custfmt`                                       |                                                              |
| `ida_bytes.isCustFmt0`                                       | `ida_bytes.is_custfmt0`                                      |                                                              |
| `ida_bytes.isCustFmt1`                                       | `ida_bytes.is_custfmt1`                                      |                                                              |
| `ida_bytes.isCustom`                                         | `ida_bytes.is_custom`                                        |                                                              |
| `ida_bytes.isData`                                           | `ida_bytes.is_data`                                          |                                                              |
| `ida_bytes.isDefArg`                                         | `ida_bytes.is_defarg`                                        |                                                              |
| `ida_bytes.isDefArg0`                                        | `ida_bytes.is_defarg0`                                       |                                                              |
| `ida_bytes.isDefArg1`                                        | `ida_bytes.is_defarg1`                                       |                                                              |
| `ida_bytes.isDouble`                                         | `ida_bytes.is_double`                                        |                                                              |
| `ida_bytes.isDwrd`                                           | `ida_bytes.is_dword`                                         |                                                              |
| `ida_bytes.isEnabled`                                        | `ida_bytes.is_mapped`                                        |                                                              |
| `ida_bytes.isEnum`                                           | `ida_bytes.is_enum`                                          |                                                              |
| `ida_bytes.isEnum0`                                          | `ida_bytes.is_enum0`                                         |                                                              |
| `ida_bytes.isEnum1`                                          | `ida_bytes.is_enum1`                                         |                                                              |
| `ida_bytes.isFloat`                                          | `ida_bytes.is_float`                                         |                                                              |
| `ida_bytes.isFloat0`                                         | `ida_bytes.is_float0`                                        |                                                              |
| `ida_bytes.isFloat1`                                         | `ida_bytes.is_float1`                                        |                                                              |
| `ida_bytes.isFlow`                                           | `ida_bytes.is_flow`                                          |                                                              |
| `ida_bytes.isFltnum`                                         | `ida_bytes.is_fltnum`                                        |                                                              |
| `ida_bytes.isFop`                                            | `ida_bytes.is_forced_operand`                                |                                                              |
| `ida_bytes.isFunc`                                           | `ida_bytes.is_func`                                          |                                                              |
| `ida_bytes.isHead`                                           | `ida_bytes.is_head`                                          |                                                              |
| `ida_bytes.isImmd`                                           | `ida_bytes.has_immd`                                         |                                                              |
| `ida_bytes.isLoaded`                                         | `ida_bytes.is_loaded`                                        |                                                              |
| `ida_bytes.isNotTail`                                        | `ida_bytes.is_not_tail`                                      |                                                              |
| `ida_bytes.isNum`                                            | `ida_bytes.is_numop`                                         |                                                              |
| `ida_bytes.isNum0`                                           | `ida_bytes.is_numop0`                                        |                                                              |
| `ida_bytes.isNum1`                                           | `ida_bytes.is_numop1`                                        |                                                              |
| `ida_bytes.isOff`                                            | `ida_bytes.is_off`                                           |                                                              |
| `ida_bytes.isOff0`                                           | `ida_bytes.is_off0`                                          |                                                              |
| `ida_bytes.isOff1`                                           | `ida_bytes.is_off1`                                          |                                                              |
| `ida_bytes.isOwrd`                                           | `ida_bytes.is_oword`                                         |                                                              |
| `ida_bytes.isPackReal`                                       | `ida_bytes.is_pack_real`                                     |                                                              |
| `ida_bytes.isQwrd`                                           | `ida_bytes.is_qword`                                         |                                                              |
| `ida_bytes.isSeg`                                            | `ida_bytes.is_seg`                                           |                                                              |
| `ida_bytes.isSeg0`                                           | `ida_bytes.is_seg0`                                          |                                                              |
| `ida_bytes.isSeg1`                                           | `ida_bytes.is_seg1`                                          |                                                              |
| `ida_bytes.isStkvar`                                         | `ida_bytes.is_stkvar`                                        |                                                              |
| `ida_bytes.isStkvar0`                                        | `ida_bytes.is_stkvar0`                                       |                                                              |
| `ida_bytes.isStkvar1`                                        | `ida_bytes.is_stkvar1`                                       |                                                              |
| `ida_bytes.isStroff`                                         | `ida_bytes.is_stroff`                                        |                                                              |
| `ida_bytes.isStroff0`                                        | `ida_bytes.is_stroff0`                                       |                                                              |
| `ida_bytes.isStroff1`                                        | `ida_bytes.is_stroff1`                                       |                                                              |
| `ida_bytes.isStruct`                                         | `ida_bytes.is_struct`                                        |                                                              |
| `ida_bytes.isTail`                                           | `ida_bytes.is_tail`                                          |                                                              |
| `ida_bytes.isTbyt`                                           | `ida_bytes.is_tbyte`                                         |                                                              |
| `ida_bytes.isUnknown`                                        | `ida_bytes.is_unknown`                                       |                                                              |
| `ida_bytes.isVoid`                                           | `ida_bytes.is_suspop`                                        |                                                              |
| `ida_bytes.isWord`                                           | `ida_bytes.is_word`                                          |                                                              |
| `ida_bytes.isYwrd`                                           | `ida_bytes.is_yword`                                         |                                                              |
| `ida_bytes.isZwrd`                                           | `ida_bytes.is_zword`                                         |                                                              |
| `ida_bytes.make_ascii_string`                                | `ida_bytes.create_strlit`                                    |                                                              |
| `ida_bytes.noExtra`                                          | `lambda *args: False`                                        |                                                              |
| `ida_bytes.noType`                                           | `ida_bytes.clr_op_type`                                      |                                                              |
| `ida_bytes.owrdflag`                                         | `ida_bytes.oword_flag`                                       |                                                              |
| `ida_bytes.patch_many_bytes`                                 | `ida_bytes.patch_bytes`                                      |                                                              |
| `ida_bytes.print_ascii_string_type`                          | `ida_bytes.print_strlit_type`                                |                                                              |
| `ida_bytes.put_many_bytes`                                   | `ida_bytes.put_bytes`                                        |                                                              |
| `ida_bytes.qwrdflag`                                         | `ida_bytes.qword_flag`                                       |                                                              |
| `ida_bytes.tbytflag`                                         | `ida_bytes.tbyte_flag`                                       |                                                              |
| `ida_bytes.update_hidden_area`                               | `ida_bytes.update_hidden_range`                              |                                                              |
| `ida_bytes.ywrdflag`                                         | `ida_bytes.yword_flag`                                       |                                                              |
| `ida_bytes.zwrdflag`                                         | `ida_bytes.zword_flag`                                       |                                                              |
| `ida_bytes.get_opinfo(ea, n, flags, buf)`                    | `ida_bytes.get_opinfo(buf, ea, n, flags)`                    |                                                              |
| `ida_bytes.doASCI(ea, length)`                               | `ida_bytes.create_data(ea, FF_STRLIT, length, ida_netnode.BADNODE)` |                                                              |
| `ida_bytes.FF_3BYTE`                                         | `ida_bytes.FF_BYTE`                                          |                                                              |
| `ida_bytes.chunksize`                                        | `ida_bytes.chunk_size`                                       |                                                              |
| `ida_bytes.chunkstart`                                       | `ida_bytes.chunk_start`                                      |                                                              |
| `ida_bytes.do3byte`                                          | `lambda *args: False`                                        |                                                              |
| `ida_bytes.f_is3byte`                                        | `lambda *args: False`                                        |                                                              |
| `ida_bytes.freechunk`                                        | `ida_bytes.free_chunk`                                       |                                                              |
| `ida_bytes.get_3byte`                                        | `lambda *args: False`                                        |                                                              |
| `ida_bytes.is3byte`                                          | `lambda *args: False`                                        |                                                              |
| `ida_bytes.nextaddr`                                         | `ida_bytes.next_addr`                                        |                                                              |
| `ida_bytes.nextchunk`                                        | `ida_bytes.next_chunk`                                       |                                                              |
| `ida_bytes.nextthat`                                         | `ida_bytes.next_that`                                        |                                                              |
| `ida_bytes.prevaddr`                                         | `ida_bytes.prev_addr`                                        |                                                              |
| `ida_bytes.prevchunk`                                        | `ida_bytes.prev_chunk`                                       |                                                              |
| `ida_bytes.prevthat`                                         | `ida_bytes.prev_that`                                        |                                                              |
| `ida_bytes.tribyteflag`                                      | `ida_bytes.byte_flag`                                        |                                                              |
| `ida_bytes.alignflag`                                        | `ida_bytes.align_flag`                                       |                                                              |
| `ida_bytes.binflag`                                          | `ida_bytes.bin_flag`                                         |                                                              |
| `ida_bytes.byteflag`                                         | `ida_bytes.byte_flag`                                        |                                                              |
| `ida_bytes.charflag`                                         | `ida_bytes.char_flag`                                        |                                                              |
| `ida_bytes.codeflag`                                         | `ida_bytes.code_flag`                                        |                                                              |
| `ida_bytes.custflag`                                         | `ida_bytes.cust_flag`                                        |                                                              |
| `ida_bytes.custfmtflag`                                      | `ida_bytes.custfmt_flag`                                     |                                                              |
| `ida_bytes.decflag`                                          | `ida_bytes.dec_flag`                                         |                                                              |
| `ida_bytes.doubleflag`                                       | `ida_bytes.double_flag`                                      |                                                              |
| `ida_bytes.enumflag`                                         | `ida_bytes.enum_flag`                                        |                                                              |
| `ida_bytes.floatflag`                                        | `ida_bytes.float_flag`                                       |                                                              |
| `ida_bytes.fltflag`                                          | `ida_bytes.flt_flag`                                         |                                                              |
| `ida_bytes.hexflag`                                          | `ida_bytes.hex_flag`                                         |                                                              |
| `ida_bytes.numflag`                                          | `ida_bytes.num_flag`                                         |                                                              |
| `ida_bytes.octflag`                                          | `ida_bytes.oct_flag`                                         |                                                              |
| `ida_bytes.offflag`                                          | `ida_bytes.off_flag`                                         |                                                              |
| `ida_bytes.packrealflag`                                     | `ida_bytes.packreal_flag`                                    |                                                              |
| `ida_bytes.segflag`                                          | `ida_bytes.seg_flag`                                         |                                                              |
| `ida_bytes.stkvarflag`                                       | `ida_bytes.stkvar_flag`                                      |                                                              |
| `ida_bytes.stroffflag`                                       | `ida_bytes.stroff_flag`                                      |                                                              |
| `ida_bytes.struflag`                                         | `ida_bytes.stru_flag`                                        |                                                              |
| `ida_bytes.wordflag`                                         | `ida_bytes.word_flag`                                        |                                                              |
| `ida_bytes.invalidate_visea_cache`                           | `lambda *args: False`                                        |                                                              |
| `ida_bytes.op_stroff(ea, n, path, path_len, delta)`          | `ida_bytes.op_stroff(insn_t, n, path, path_len, delta)`      |                                                              |
| `ida_bytes.doVar`                                            | `removed; no substitution`                                   |                                                              |
| `ida_idaapi.pycim_get_tcustom_control`                       | `ida_idaapi.pycim_get_widget`                                |                                                              |
| `ida_idaapi.pycim_get_tform`                                 | `ida_idaapi.pycim_get_widget`                                |                                                              |
| `ida_hexrays.get_tform_vdui`                                 | `ida_hexrays.get_widget_vdui`                                |                                                              |
| `ida_hexrays.hx_get_tform_vdui`                              | `ida_hexrays.hx_get_widget_vdui`                             |                                                              |



### idc.py

The following table concerns the `idc.py` module, where a lot of the compatibility layer was removed.

Note:

- when the `before` and `after` have no parentheses, it means they take the exact same parameters.
- when the `before` and `after` have parentheses, it means they take somewhat different parameters, and thus one has to be careful when porting
- as you will see, quite a few functions that were present in the `idc` module, have a replacement directly in another, upstream `ida_*` module.



| Before                                    | After                                                        | Notes |
| ----------------------------------------- | ------------------------------------------------------------ | ----- |
| `idc.GetString`                           | `ida_bytes.get_strlit_contents`                              |       |
| `idc.GetRegValue`                         | `idc.get_reg_value`                                          |       |
| `idc.LocByName`                           | `idc.get_name_ea_simple`                                     |       |
| `idc.AddBpt`                              | `idc.add_bpt`                                                |       |
| `idc.Compile(file)`                       | `idc.CompileEx(file, 1)`                                     |       |
| `idc.CompileEx(input, is_file)`           | `idc.compile_idc_file(input) if is_file else compile_idc_text(input)` |       |
| `idc.OpOffset(ea, base)`                  | `idc.op_plain_offset(ea, -1, base)`                          |       |
| `idc.OpNum(ea)`                           | `idc.op_num(ea, -1)`                                         |       |
| `idc.OpChar(ea)`                          | `idc.op_chr(ea, -1)`                                         |       |
| `idc.OpSegment(ea)`                       | `idc.op_seg(ea, -1)`                                         |       |
| `idc.OpDec(ea)`                           | `idc.op_dec(ea, -1)`                                         |       |
| `idc.OpAlt1(ea, str)`                     | `idc.op_man(ea, 0, str)`                                     |       |
| `idc.OpAlt2(ea, str)`                     | `idc.op_man(ea, 1, str)`                                     |       |
| `idc.StringStp(x)`                        | `idc.set_inf_attr(INF_STRLIT_BREAK, x)`                      |       |
| `idc.LowVoids(x)`                         | `idc.set_inf_attr(INF_LOW_OFF, x)`                           |       |
| `idc.HighVoids(x)`                        | `idc.set_inf_attr(INF_HIGH_OFF, x)`                          |       |
| `idc.TailDepth(x)`                        | `idc.set_inf_attr(INF_MAXREF, x)`                            |       |
| `idc.Analysis(x)`                         | `idc.set_flag(INF_GENFLAGS, INFFL_AUTO, x)`                  |       |
| `idc.Comments(x)`                         | `idc.set_flag(INF_CMTFLAG, SW_ALLCMT, x)`                    |       |
| `idc.Voids(x)`                            | `idc.set_flag(INF_OUTFLAGS, OFLG_SHOW_VOID, x)`              |       |
| `idc.XrefShow(x)`                         | `idc.set_inf_attr(INF_XREFNUM, x)`                           |       |
| `idc.Indent(x)`                           | `idc.set_inf_attr(INF_INDENT, x)`                            |       |
| `idc.CmtIndent(x)`                        | `idc.set_inf_attr(INF_COMMENT, x)`                           |       |
| `idc.AutoShow(x)`                         | `idc.set_flag(INF_OUTFLAGS, OFLG_SHOW_AUTO, x)`              |       |
| `idc.MinEA()`                             | `ida_ida.inf_get_min_ea()`                                   |       |
| `idc.MaxEA()`                             | `ida_ida.inf_get_max_ea()`                                   |       |
| `idc.StartEA()`                           | `ida_ida.inf_get_min_ea()`                                   |       |
| `idc.BeginEA()`                           | `ida_ida.inf_get_min_ea()`                                   |       |
| `idc.set_start_cs(x)`                     | `idc.set_inf_attr(INF_START_CS, x)`                          |       |
| `idc.set_start_ip(x)`                     | `idc.set_inf_attr(INF_START_IP, x)`                          |       |
| `idc.auto_make_code(x)`                   | `idc.auto_mark_range(x, (x)+1, AU_CODE);`                    |       |
| `idc.AddConst(enum_id, name, value)`      | `idc.add_enum_member(enum_id, name, value, -1)`              |       |
| `idc.AddStruc(index, name)`               | `idc.add_struc(index, name, 0)`                              |       |
| `idc.AddUnion(index, name)`               | `idc.add_struc(index, name, 1)`                              |       |
| `idc.OpStroff(ea, n, strid)`              | `idc.op_stroff(ea, n, strid, 0)`                             |       |
| `idc.OpEnum(ea, n, enumid)`               | `idc.op_enum(ea, n, enumid, 0)`                              |       |
| `idc.DelConst(id, v, mask)`               | `idc.del_enum_member(id, v, 0, mask)`                        |       |
| `idc.GetConst(id, v, mask)`               | `idc.get_enum_member(id, v, 0, mask)`                        |       |
| `idc.AnalyseRange`                        | `idc.plan_and_wait`                                          |       |
| `idc.AnalyseArea`                         | `idc.plan_and_wait`                                          |       |
| `idc.AnalyzeArea`                         | `idc.plan_and_wait`                                          |       |
| `idc.MakeStruct(ea, name)`                | `idc.create_struct(ea, -1, name)`                            |       |
| `idc.Name(ea)`                            | `idc.get_name(ea, ida_name.GN_VISIBLE)`                      |       |
| `idc.GetTrueName`                         | `ida_name.get_ea_name`                                       |       |
| `idc.MakeName(ea, name)`                  | `idc.set_name(ea, name, SN_CHECK)`                           |       |
| `idc.GetFrame(ea)`                        | `idc.get_func_attr(ea, FUNCATTR_FRAME)`                      |       |
| `idc.GetFrameLvarSize(ea)`                | `idc.get_func_attr(ea, FUNCATTR_FRSIZE)`                     |       |
| `idc.GetFrameRegsSize(ea)`                | `idc.get_func_attr(ea, FUNCATTR_FRREGS)`                     |       |
| `idc.GetFrameArgsSize(ea)`                | `idc.get_func_attr(ea, FUNCATTR_ARGSIZE)`                    |       |
| `idc.GetFunctionFlags(ea)`                | `idc.get_func_attr(ea, FUNCATTR_FLAGS)`                      |       |
| `idc.SetFunctionFlags(ea, flags)`         | `idc.set_func_attr(ea, FUNCATTR_FLAGS, flags)`               |       |
| `idc.SegCreate`                           | `idc.AddSeg`                                                 |       |
| `idc.SegDelete`                           | `idc.del_segm`                                               |       |
| `idc.SegBounds`                           | `idc.set_segment_bounds`                                     |       |
| `idc.SegRename`                           | `idc.set_segm_name`                                          |       |
| `idc.SegClass`                            | `idc.set_segm_class`                                         |       |
| `idc.SegAddrng`                           | `idc.set_segm_addressing`                                    |       |
| `idc.SegDefReg`                           | `idc.set_default_sreg_value`                                 |       |
| `idc.Comment(ea)`                         | `idc.get_cmt(ea, 0)`                                         |       |
| `idc.RptCmt(ea)`                          | `idc.get_cmt(ea, 1)`                                         |       |
| `idc.MakeByte(ea)`                        | `ida_bytes.create_data(ea, FF_BYTE, 1, ida_idaapi.BADADDR)`  |       |
| `idc.MakeWord(ea)`                        | `ida_bytes.create_data(ea, FF_WORD, 2, ida_idaapi.BADADDR)`  |       |
| `idc.MakeDword(ea)`                       | `ida_bytes.create_data(ea, FF_DWORD, 4, ida_idaapi.BADADDR)` |       |
| `idc.MakeQword(ea)`                       | `ida_bytes.create_data(ea, FF_QWORD, 8, ida_idaapi.BADADDR)` |       |
| `idc.MakeOword(ea)`                       | `ida_bytes.create_data(ea, FF_OWORD, 16, ida_idaapi.BADADDR)` |       |
| `idc.MakeYword(ea)`                       | `ida_bytes.create_data(ea, FF_YWORD, 32, ida_idaapi.BADADDR)` |       |
| `idc.MakeFloat(ea)`                       | `ida_bytes.create_data(ea, FF_FLOAT, 4, ida_idaapi.BADADDR)` |       |
| `idc.MakeDouble(ea)`                      | `ida_bytes.create_data(ea, FF_DOUBLE, 8, ida_idaapi.BADADDR)` |       |
| `idc.MakePackReal(ea)`                    | `ida_bytes.create_data(ea, FF_PACKREAL, 10, ida_idaapi.BADADDR)` |       |
| `idc.MakeTbyte(ea)`                       | `ida_bytes.create_data(ea, FF_TBYTE, 10, ida_idaapi.BADADDR)` |       |
| `idc.MakeCustomData(ea, size, dtid, fid)` | `ida_bytes.create_data(ea, FF_CUSTOM, size, dtid|((fid)<<16))` |       |
| `idc.SetReg(ea, reg, value)`              | `idc.split_sreg_range(ea, reg, value, SR_user)`              |       |
| `idc.SegByName`                           | `idc.selector_by_name`                                       |       |
| `idc.MK_FP`                               | `idc.to_ea`                                                  |       |
| `idc.toEA`                                | `idc.to_ea`                                                  |       |
| `idc.MakeCode`                            | `idc.create_insn`                                            |       |
| `idc.MakeNameEx`                          | `idc.set_name`                                               |       |
| `idc.MakeArray`                           | `idc.make_array`                                             |       |
| `idc.MakeData`                            | `ida_bytes.create_data`                                      |       |
| `idc.GetRegValue`                         | `idc.get_reg_value`                                          |       |
| `idc.SetRegValue`                         | `idc.set_reg_value`                                          |       |
| `idc.Byte`                                | `idc.get_wide_byte`                                          |       |
| `idc.Word`                                | `idc.get_wide_word`                                          |       |
| `idc.Dword`                               | `idc.get_wide_dword`                                         |       |
| `idc.Qword`                               | `idc.get_qword`                                              |       |
| `idc.LocByName`                           | `idc.get_name_ea_simple`                                     |       |
| `idc.ScreenEA`                            | `idc.get_screen_ea`                                          |       |
| `idc.GetTinfo`                            | `idc.get_tinfo`                                              |       |
| `idc.OpChr`                               | `idc.op_chr`                                                 |       |
| `idc.OpSeg`                               | `idc.op_seg`                                                 |       |
| `idc.OpNumber`                            | `idc.op_num`                                                 |       |
| `idc.OpDecimal`                           | `idc.op_dec`                                                 |       |
| `idc.OpOctal`                             | `idc.op_oct`                                                 |       |
| `idc.OpBinary`                            | `idc.op_bin`                                                 |       |
| `idc.OpHex`                               | `idc.op_hex`                                                 |       |
| `idc.OpAlt`                               | `idc.op_man`                                                 |       |
| `idc.OpSign`                              | `idc.toggle_sign`                                            |       |
| `idc.OpNot`                               | `idc.toggle_bnot`                                            |       |
| `idc.OpEnumEx`                            | `idc.op_enum`                                                |       |
| `idc.OpStroffEx`                          | `idc.op_stroff`                                              |       |
| `idc.OpStkvar`                            | `idc.op_stkvar`                                              |       |
| `idc.OpFloat`                             | `idc.op_flt`                                                 |       |
| `idc.OpOffEx`                             | `idc.op_offset`                                              |       |
| `idc.OpOff`                               | `idc.op_plain_offset`                                        |       |
| `idc.MakeStructEx`                        | `idc.create_struct`                                          |       |
| `idc.Jump`                                | `ida_kernwin.jumpto`                                         |       |
| `idc.GenerateFile`                        | `idc.gen_file`                                               |       |
| `idc.GenFuncGdl`                          | `idc.gen_flow_graph`                                         |       |
| `idc.GenCallGdl`                          | `idc.gen_simple_call_chart`                                  |       |
| `idc.IdbByte`                             | `ida_bytes.get_db_byte`                                      |       |
| `idc.DbgByte`                             | `idc.read_dbg_byte`                                          |       |
| `idc.DbgWord`                             | `idc.read_dbg_word`                                          |       |
| `idc.DbgDword`                            | `idc.read_dbg_dword`                                         |       |
| `idc.DbgQword`                            | `idc.read_dbg_qword`                                         |       |
| `idc.DbgRead`                             | `idc.read_dbg_memory`                                        |       |
| `idc.DbgWrite`                            | `idc.write_dbg_memory`                                       |       |
| `idc.PatchDbgByte`                        | `idc.patch_dbg_byte`                                         |       |
| `idc.PatchByte`                           | `ida_bytes.patch_byte`                                       |       |
| `idc.PatchWord`                           | `ida_bytes.patch_word`                                       |       |
| `idc.PatchDword`                          | `ida_bytes.patch_dword`                                      |       |
| `idc.PatchQword`                          | `ida_bytes.patch_qword`                                      |       |
| `idc.SetProcessorType`                    | `ida_idp.set_processor_type`                                 |       |
| `idc.SetTargetAssembler`                  | `ida_idp.set_target_assembler`                               |       |
| `idc.Batch`                               | `idc.batch`                                                  |       |
| `idc.SetSegDefReg`                        | `idc.set_default_sreg_value`                                 |       |
| `idc.GetReg`                              | `idc.get_sreg`                                               |       |
| `idc.SetRegEx`                            | `idc.split_sreg_range`                                       |       |
| `idc.WriteMap(path)`                      | `idc.gen_file(OFILE_MAP, path, 0, BADADDR, GENFLG_MAPSEG|GENFLG_MAPNAME)` |       |
| `idc.WriteTxt(path, ea1, ea2)`            | `idc.gen_file(OFILE_ASM, path, ea1, ea2, 0)`                 |       |
| `idc.WriteExe(path)`                      | `idc.gen_file(OFILE_EXE, path, 0, BADADDR, 0)`               |       |
| `idc.AskStr(defval, prompt)`              | `ida_kernwin.ask_str(defval, 0, prompt)`                     |       |
| `idc.AskFile`                             | `ida_kernwin.ask_file`                                       |       |
| `idc.AskAddr`                             | `ida_kernwin.ask_addr`                                       |       |
| `idc.AskLong`                             | `ida_kernwin.ask_long`                                       |       |
| `idc.AskSeg`                              | `ida_kernwin.ask_seg`                                        |       |
| `idc.AskIdent(defval, prompt)`            | `ida_kernwin.ask_str(defval, ida_kernwin.HIST_IDENT, prompt)` |       |
| `idc.AskYN`                               | `ida_kernwin.ask_yn`                                         |       |
| `idc.DeleteAll`                           | `idc.delete_all_segments`                                    |       |
| `idc.AddSegEx`                            | `idc.add_segm_ex`                                            |       |
| `idc.SetSegBounds`                        | `idc.set_segment_bounds`                                     |       |
| `idc.RenameSeg`                           | `idc.set_segm_name`                                          |       |
| `idc.SetSegClass`                         | `idc.set_segm_class`                                         |       |
| `idc.SetSegAddressing`                    | `idc.set_segm_addressing`                                    |       |
| `idc.SetSegmentAttr`                      | `idc.set_segm_attr`                                          |       |
| `idc.GetSegmentAttr`                      | `idc.get_segm_attr`                                          |       |
| `idc.SetStorageType`                      | `ida_bytes.change_storage_type`                              |       |
| `idc.MoveSegm`                            | `idc.move_segm`                                              |       |
| `idc.RebaseProgram`                       | `ida_segment.rebase_program`                                 |       |
| `idc.GetNsecStamp`                        | `idc.get_nsec_stamp`                                         |       |
| `idc.LocByNameEx`                         | `ida_name.get_name_ea`                                       |       |
| `idc.SegByBase`                           | `idc.get_segm_by_sel`                                        |       |
| `idc.GetCurrentLine`                      | `idc.get_curline`                                            |       |
| `idc.SelStart`                            | `idc.read_selection_start`                                   |       |
| `idc.SelEnd`                              | `idc.read_selection_end`                                     |       |
| `idc.FirstSeg`                            | `idc.get_first_seg`                                          |       |
| `idc.NextSeg`                             | `idc.get_next_seg`                                           |       |
| `idc.SegName`                             | `idc.get_segm_name`                                          |       |
| `idc.CommentEx`                           | `ida_bytes.get_cmt`                                          |       |
| `idc.AltOp`                               | `ida_bytes.get_forced_operand`                               |       |
| `idc.GetDisasmEx`                         | `idc.generate_disasm_line`                                   |       |
| `idc.GetMnem`                             | `idc.print_insn_mnem`                                        |       |
| `idc.GetOpType`                           | `idc.get_operand_type`                                       |       |
| `idc.GetOperandValue`                     | `idc.get_operand_value`                                      |       |
| `idc.DecodeInstruction`                   | `ida_ua.decode_insn`                                         |       |
| `idc.NextAddr`                            | `ida_bytes.next_addr`                                        |       |
| `idc.PrevAddr`                            | `ida_bytes.prev_addr`                                        |       |
| `idc.NextNotTail`                         | `ida_bytes.next_not_tail`                                    |       |
| `idc.PrevNotTail`                         | `ida_bytes.prev_not_tail`                                    |       |
| `idc.ItemHead`                            | `ida_bytes.get_item_head`                                    |       |
| `idc.ItemEnd`                             | `ida_bytes.get_item_end`                                     |       |
| `idc.ItemSize`                            | `idc.get_item_size`                                          |       |
| `idc.AnalyzeRange`                        | `idc.plan_and_wait`                                          |       |
| `idc.ExecIDC`                             | `idc.exec_idc`                                               |       |
| `idc.Eval`                                | `idc.eval_idc`                                               |       |
| `idc.Exit`                                | `ida_pro.qexit`                                              |       |
| `idc.FindVoid`                            | `ida_search.find_suspop`                                     |       |
| `idc.FindCode`                            | `ida_search.find_code`                                       |       |
| `idc.FindData`                            | `ida_search.find_data`                                       |       |
| `idc.FindUnexplored`                      | `ida_search.find_unknown`                                    |       |
| `idc.FindExplored`                        | `ida_search.find_defined`                                    |       |
| `idc.FindImmediate`                       | `ida_search.find_imm`                                        |       |
| `idc.AddCodeXref`                         | `ida_xref.add_cref`                                          |       |
| `idc.DelCodeXref`                         | `ida_xref.del_cref`                                          |       |
| `idc.Rfirst`                              | `ida_xref.get_first_cref_from`                               |       |
| `idc.RfirstB`                             | `ida_xref.get_first_cref_to`                                 |       |
| `idc.Rnext`                               | `ida_xref.get_next_cref_from`                                |       |
| `idc.RnextB`                              | `ida_xref.get_next_cref_to`                                  |       |
| `idc.Rfirst0`                             | `ida_xref.get_first_fcref_from`                              |       |
| `idc.RfirstB0`                            | `ida_xref.get_first_fcref_to`                                |       |
| `idc.Rnext0`                              | `ida_xref.get_next_fcref_from`                               |       |
| `idc.RnextB0`                             | `ida_xref.get_next_fcref_to`                                 |       |
| `idc.Dfirst`                              | `ida_xref.get_first_dref_from`                               |       |
| `idc.Dnext`                               | `ida_xref.get_next_dref_from`                                |       |
| `idc.DfirstB`                             | `ida_xref.get_first_dref_to`                                 |       |
| `idc.DnextB`                              | `ida_xref.get_next_dref_to`                                  |       |
| `idc.XrefType`                            | `idc.get_xref_type`                                          |       |
| `idc.AutoUnmark`                          | `ida_auto.auto_unmark`                                       |       |
| `idc.AutoMark2`                           | `ida_auto.auto_mark_range`                                   |       |
| `idc.SetSelector`                         | `ida_segment.set_selector`                                   |       |
| `idc.AskSelector`                         | `idc.sel2para`                                               |       |
| `idc.ask_selector`                        | `idc.sel2para`                                               |       |
| `idc.FindSelector`                        | `idc.find_selector`                                          |       |
| `idc.DelSelector`                         | `ida_segment.del_selector`                                   |       |
| `idc.MakeFunction`                        | `ida_funcs.add_func`                                         |       |
| `idc.DelFunction`                         | `ida_funcs.del_func`                                         |       |
| `idc.SetFunctionEnd`                      | `ida_funcs.set_func_end`                                     |       |
| `idc.NextFunction`                        | `idc.get_next_func`                                          |       |
| `idc.PrevFunction`                        | `idc.get_prev_func`                                          |       |
| `idc.GetFunctionAttr`                     | `idc.get_func_attr`                                          |       |
| `idc.SetFunctionAttr`                     | `idc.set_func_attr`                                          |       |
| `idc.GetFunctionName`                     | `idc.get_func_name`                                          |       |
| `idc.GetFunctionCmt`                      | `idc.get_func_cmt`                                           |       |
| `idc.SetFunctionCmt`                      | `idc.set_func_cmt`                                           |       |
| `idc.ChooseFunction`                      | `idc.choose_func`                                            |       |
| `idc.GetFuncOffset`                       | `idc.get_func_off_str`                                       |       |
| `idc.MakeLocal`                           | `idc.define_local_var`                                       |       |
| `idc.FindFuncEnd`                         | `idc.find_func_end`                                          |       |
| `idc.GetFrameSize`                        | `idc.get_frame_size`                                         |       |
| `idc.MakeFrame`                           | `idc.set_frame_size`                                         |       |
| `idc.GetSpd`                              | `idc.get_spd`                                                |       |
| `idc.GetSpDiff`                           | `idc.get_sp_delta`                                           |       |
| `idc.DelStkPnt`                           | `idc.del_stkpnt`                                             |       |
| `idc.AddAutoStkPnt2`                      | `idc.add_auto_stkpnt`                                        |       |
| `idc.RecalcSpd`                           | `ida_frame.recalc_spd`                                       |       |
| `idc.GetMinSpd`                           | `idc.get_min_spd_ea`                                         |       |
| `idc.GetFchunkAttr`                       | `idc.get_fchunk_attr`                                        |       |
| `idc.SetFchunkAttr`                       | `idc.set_fchunk_attr`                                        |       |
| `idc.GetFchunkReferer`                    | `ida_funcs.get_fchunk_referer`                               |       |
| `idc.NextFchunk`                          | `idc.get_next_fchunk`                                        |       |
| `idc.PrevFchunk`                          | `idc.get_prev_fchunk`                                        |       |
| `idc.AppendFchunk`                        | `idc.append_func_tail`                                       |       |
| `idc.RemoveFchunk`                        | `idc.remove_fchunk`                                          |       |
| `idc.SetFchunkOwner`                      | `idc.set_tail_owner`                                         |       |
| `idc.FirstFuncFchunk`                     | `idc.first_func_chunk`                                       |       |
| `idc.NextFuncFchunk`                      | `idc.next_func_chunk`                                        |       |
| `idc.GetEntryPointQty`                    | `ida_entry.get_entry_qty`                                    |       |
| `idc.AddEntryPoint`                       | `ida_entry.add_entry`                                        |       |
| `idc.GetEntryName`                        | `ida_entry.get_entry_name`                                   |       |
| `idc.GetEntryOrdinal`                     | `ida_entry.get_entry_ordinal`                                |       |
| `idc.GetEntryPoint`                       | `ida_entry.get_entry`                                        |       |
| `idc.RenameEntryPoint`                    | `ida_entry.rename_entry`                                     |       |
| `idc.GetNextFixupEA`                      | `ida_fixup.get_next_fixup_ea`                                |       |
| `idc.GetPrevFixupEA`                      | `ida_fixup.get_prev_fixup_ea`                                |       |
| `idc.GetFixupTgtType`                     | `idc.get_fixup_target_type`                                  |       |
| `idc.GetFixupTgtFlags`                    | `idc.get_fixup_target_flags`                                 |       |
| `idc.GetFixupTgtSel`                      | `idc.get_fixup_target_sel`                                   |       |
| `idc.GetFixupTgtOff`                      | `idc.get_fixup_target_off`                                   |       |
| `idc.GetFixupTgtDispl`                    | `idc.get_fixup_target_dis`                                   |       |
| `idc.SetFixup`                            | `idc.set_fixup`                                              |       |
| `idc.DelFixup`                            | `ida_fixup.del_fixup`                                        |       |
| `idc.MarkPosition`                        | `idc.put_bookmark`                                           |       |
| `idc.GetMarkedPos`                        | `idc.get_bookmark`                                           |       |
| `idc.GetMarkComment`                      | `idc.get_bookmark_desc`                                      |       |
| `idc.GetStrucQty`                         | `ida_struct.get_struc_qty`                                   |       |
| `idc.GetFirstStrucIdx`                    | `ida_struct.get_first_struc_idx`                             |       |
| `idc.GetLastStrucIdx`                     | `ida_struct.get_last_struc_idx`                              |       |
| `idc.GetNextStrucIdx`                     | `ida_struct.get_next_struc_idx`                              |       |
| `idc.GetPrevStrucIdx`                     | `ida_struct.get_prev_struc_idx`                              |       |
| `idc.GetStrucIdx`                         | `ida_struct.get_struc_idx`                                   |       |
| `idc.GetStrucId`                          | `ida_struct.get_struc_by_idx`                                |       |
| `idc.GetStrucIdByName`                    | `ida_struct.get_struc_id`                                    |       |
| `idc.GetStrucName`                        | `ida_struct.get_struc_name`                                  |       |
| `idc.GetStrucComment`                     | `ida_struct.get_struc_cmt`                                   |       |
| `idc.GetStrucSize`                        | `ida_struct.get_struc_size`                                  |       |
| `idc.GetMemberQty`                        | `idc.get_member_qty`                                         |       |
| `idc.GetStrucPrevOff`                     | `idc.get_prev_offset`                                        |       |
| `idc.GetStrucNextOff`                     | `idc.get_next_offset`                                        |       |
| `idc.GetFirstMember`                      | `idc.get_first_member`                                       |       |
| `idc.GetLastMember`                       | `idc.get_last_member`                                        |       |
| `idc.GetMemberOffset`                     | `idc.get_member_offset`                                      |       |
| `idc.GetMemberName`                       | `idc.get_member_name`                                        |       |
| `idc.GetMemberComment`                    | `idc.get_member_cmt`                                         |       |
| `idc.GetMemberSize`                       | `idc.get_member_size`                                        |       |
| `idc.GetMemberFlag`                       | `idc.get_member_flag`                                        |       |
| `idc.GetMemberStrId`                      | `idc.get_member_strid`                                       |       |
| `idc.GetMemberId`                         | `idc.get_member_id`                                          |       |
| `idc.AddStrucEx`                          | `idc.add_struc`                                              |       |
| `idc.IsUnion`                             | `idc.is_union`                                               |       |
| `idc.DelStruc`                            | `idc.del_struc`                                              |       |
| `idc.SetStrucIdx`                         | `idc.set_struc_idx`                                          |       |
| `idc.SetStrucName`                        | `ida_struct.set_struc_name`                                  |       |
| `idc.SetStrucComment`                     | `ida_struct.set_struc_cmt`                                   |       |
| `idc.SetStrucAlign`                       | `idc.set_struc_align`                                        |       |
| `idc.AddStrucMember`                      | `idc.add_struc_member`                                       |       |
| `idc.DelStrucMember`                      | `idc.del_struc_member`                                       |       |
| `idc.SetMemberName`                       | `idc.set_member_name`                                        |       |
| `idc.SetMemberType`                       | `idc.set_member_type`                                        |       |
| `idc.SetMemberComment`                    | `idc.set_member_cmt`                                         |       |
| `idc.ExpandStruc`                         | `idc.expand_struc`                                           |       |
| `idc.SetLineNumber`                       | `ida_nalt.set_source_linnum`                                 |       |
| `idc.GetLineNumber`                       | `ida_nalt.get_source_linnum`                                 |       |
| `idc.DelLineNumber`                       | `ida_nalt.del_source_linnum`                                 |       |
| `idc.AddSourceFile`                       | `ida_lines.add_sourcefile`                                   |       |
| `idc.GetSourceFile`                       | `ida_lines.get_sourcefile`                                   |       |
| `idc.DelSourceFile`                       | `ida_lines.del_sourcefile`                                   |       |
| `idc.CreateArray`                         | `idc.create_array`                                           |       |
| `idc.GetArrayId`                          | `idc.get_array_id`                                           |       |
| `idc.RenameArray`                         | `idc.rename_array`                                           |       |
| `idc.DeleteArray`                         | `idc.delete_array`                                           |       |
| `idc.SetArrayLong`                        | `idc.set_array_long`                                         |       |
| `idc.SetArrayString`                      | `idc.set_array_string`                                       |       |
| `idc.GetArrayElement`                     | `idc.get_array_element`                                      |       |
| `idc.DelArrayElement`                     | `idc.del_array_element`                                      |       |
| `idc.GetFirstIndex`                       | `idc.get_first_index`                                        |       |
| `idc.GetNextIndex`                        | `idc.get_next_index`                                         |       |
| `idc.GetLastIndex`                        | `idc.get_last_index`                                         |       |
| `idc.GetPrevIndex`                        | `idc.get_prev_index`                                         |       |
| `idc.SetHashLong`                         | `idc.set_hash_long`                                          |       |
| `idc.SetHashString`                       | `idc.set_hash_string`                                        |       |
| `idc.GetHashLong`                         | `idc.get_hash_long`                                          |       |
| `idc.GetHashString`                       | `idc.get_hash_string`                                        |       |
| `idc.DelHashElement`                      | `idc.del_hash_string`                                        |       |
| `idc.GetFirstHashKey`                     | `idc.get_first_hash_key`                                     |       |
| `idc.GetNextHashKey`                      | `idc.get_next_hash_key`                                      |       |
| `idc.GetLastHashKey`                      | `idc.get_last_hash_key`                                      |       |
| `idc.GetPrevHashKey`                      | `idc.get_prev_hash_key`                                      |       |
| `idc.GetEnumQty`                          | `ida_enum.get_enum_qty`                                      |       |
| `idc.GetnEnum`                            | `ida_enum.getn_enum`                                         |       |
| `idc.GetEnumIdx`                          | `ida_enum.get_enum_idx`                                      |       |
| `idc.GetEnum`                             | `ida_enum.get_enum`                                          |       |
| `idc.GetEnumName`                         | `ida_enum.get_enum_name`                                     |       |
| `idc.GetEnumCmt`                          | `ida_enum.get_enum_cmt`                                      |       |
| `idc.GetEnumSize`                         | `ida_enum.get_enum_size`                                     |       |
| `idc.GetEnumWidth`                        | `ida_enum.get_enum_width`                                    |       |
| `idc.GetEnumFlag`                         | `ida_enum.get_enum_flag`                                     |       |
| `idc.GetConstByName`                      | `ida_enum.get_enum_member_by_name`                           |       |
| `idc.GetConstValue`                       | `ida_enum.get_enum_member_value`                             |       |
| `idc.GetConstBmask`                       | `ida_enum.get_enum_member_bmask`                             |       |
| `idc.GetConstEnum`                        | `ida_enum.get_enum_member_enum`                              |       |
| `idc.GetConstEx`                          | `idc.get_enum_member`                                        |       |
| `idc.GetFirstBmask`                       | `ida_enum.get_first_bmask`                                   |       |
| `idc.GetLastBmask`                        | `ida_enum.get_last_bmask`                                    |       |
| `idc.GetNextBmask`                        | `ida_enum.get_next_bmask`                                    |       |
| `idc.GetPrevBmask`                        | `ida_enum.get_prev_bmask`                                    |       |
| `idc.GetFirstConst`                       | `idc.get_first_enum_member`                                  |       |
| `idc.GetLastConst`                        | `idc.get_last_enum_member`                                   |       |
| `idc.GetNextConst`                        | `idc.get_next_enum_member`                                   |       |
| `idc.GetPrevConst`                        | `idc.get_prev_enum_member`                                   |       |
| `idc.GetConstName`                        | `idc.get_enum_member_name`                                   |       |
| `idc.GetConstCmt`                         | `idc.get_enum_member_cmt`                                    |       |
| `idc.AddEnum`                             | `idc.add_enum`                                               |       |
| `idc.DelEnum`                             | `ida_enum.del_enum`                                          |       |
| `idc.SetEnumIdx`                          | `ida_enum.set_enum_idx`                                      |       |
| `idc.SetEnumName`                         | `ida_enum.set_enum_name`                                     |       |
| `idc.SetEnumCmt`                          | `ida_enum.set_enum_cmt`                                      |       |
| `idc.SetEnumFlag`                         | `ida_enum.set_enum_flag`                                     |       |
| `idc.SetEnumWidth`                        | `ida_enum.set_enum_width`                                    |       |
| `idc.SetEnumBf`                           | `ida_enum.set_enum_bf`                                       |       |
| `idc.AddConstEx`                          | `idc.add_enum_member`                                        |       |
| `idc.DelConstEx`                          | `idc.del_enum_member`                                        |       |
| `idc.SetConstName`                        | `ida_enum.set_enum_member_name`                              |       |
| `idc.SetConstCmt`                         | `ida_enum.set_enum_member_cmt`                               |       |
| `idc.IsBitfield`                          | `ida_enum.is_bf`                                             |       |
| `idc.SetBmaskName`                        | `idc.set_bmask_name`                                         |       |
| `idc.GetBmaskName`                        | `idc.get_bmask_name`                                         |       |
| `idc.SetBmaskCmt`                         | `idc.set_bmask_cmt`                                          |       |
| `idc.GetBmaskCmt`                         | `idc.get_bmask_cmt`                                          |       |
| `idc.GetLongPrm`                          | `idc.get_inf_attr`                                           |       |
| `idc.GetShortPrm`                         | `idc.get_inf_attr`                                           |       |
| `idc.GetCharPrm`                          | `idc.get_inf_attr`                                           |       |
| `idc.SetLongPrm`                          | `idc.set_inf_attr`                                           |       |
| `idc.SetShortPrm`                         | `idc.set_inf_attr`                                           |       |
| `idc.SetCharPrm`                          | `idc.set_inf_attr`                                           |       |
| `idc.ChangeConfig`                        | `idc.process_config_line`                                    |       |
| `idc.AddHotkey`                           | `ida_kernwin.add_idc_hotkey`                                 |       |
| `idc.DelHotkey`                           | `ida_kernwin.del_idc_hotkey`                                 |       |
| `idc.GetInputFile`                        | `ida_nalt.get_root_filename`                                 |       |
| `idc.GetInputFilePath`                    | `ida_nalt.get_input_file_path`                               |       |
| `idc.SetInputFilePath`                    | `ida_nalt.set_root_filename`                                 |       |
| `idc.GetInputFileSize`                    | `idc.retrieve_input_file_size`                               |       |
| `idc.Exec`                                | `idc.call_system`                                            |       |
| `idc.Sleep`                               | `idc.qsleep`                                                 |       |
| `idc.GetIdaDirectory`                     | `idc.idadir`                                                 |       |
| `idc.GetIdbPath`                          | `idc.get_idb_path`                                           |       |
| `idc.GetInputMD5`                         | `ida_nalt.retrieve_input_file_md5`                           |       |
| `idc.OpHigh`                              | `idc.op_offset_high16`                                       |       |
| `idc.MakeAlign`                           | `ida_bytes.create_align`                                     |       |
| `idc.Demangle`                            | `idc.demangle_name`                                          |       |
| `idc.SetManualInsn`                       | `ida_bytes.set_manual_insn`                                  |       |
| `idc.GetManualInsn`                       | `ida_bytes.get_manual_insn`                                  |       |
| `idc.SetArrayFormat`                      | `idc.set_array_params`                                       |       |
| `idc.LoadTil`                             | `idc.add_default_til`                                        |       |
| `idc.Til2Idb`                             | `idc.import_type`                                            |       |
| `idc.GetMaxLocalType`                     | `idc.get_ordinal_qty`                                        |       |
| `idc.SetLocalType`                        | `idc.set_local_type`                                         |       |
| `idc.GetLocalTinfo`                       | `idc.get_local_tinfo`                                        |       |
| `idc.GetLocalTypeName`                    | `idc.get_numbered_type_name`                                 |       |
| `idc.PrintLocalTypes`                     | `idc.print_decls`                                            |       |
| `idc.SetStatus`                           | `ida_auto.set_ida_state`                                     |       |
| `idc.Refresh`                             | `ida_kernwin.refresh_idaview_anyway`                         |       |
| `idc.RefreshLists`                        | `ida_kernwin.refresh_choosers`                               |       |
| `idc.RunPlugin`                           | `ida_loader.load_and_run_plugin`                             |       |
| `idc.ApplySig`                            | `ida_funcs.plan_to_apply_idasgn`                             |       |
| `idc.ApplyType`                           | `idc.apply_type`                                             |       |
| `idc.GetStringType`                       | `idc.get_str_type`                                           |       |
| `idc.GetOriginalByte`                     | `ida_bytes.get_original_byte`                                |       |
| `idc.HideRange`                           | `ida_bytes.add_hidden_range`                                 |       |
| `idc.SetHiddenRange`                      | `idc.update_hidden_range`                                    |       |
| `idc.DelHiddenRange`                      | `ida_bytes.del_hidden_range`                                 |       |
| `idc.DelHiddenArea`                       | `ida_bytes.del_hidden_range`                                 |       |
| `idc.GetType`                             | `idc.get_type`                                               |       |
| `idc.GuessType`                           | `idc.guess_type`                                             |       |
| `idc.ParseType`                           | `idc.parse_decl`                                             |       |
| `idc.ParseTypes`                          | `idc.parse_decls`                                            |       |
| `idc.GetColor`                            | `idc.get_color`                                              |       |
| `idc.SetColor`                            | `idc.set_color`                                              |       |
| `idc.GetBptQty`                           | `ida_dbg.get_bpt_qty`                                        |       |
| `idc.GetBptEA`                            | `idc.get_bpt_ea`                                             |       |
| `idc.GetBptAttr`                          | `idc.get_bpt_attr`                                           |       |
| `idc.SetBptAttr`                          | `idc.set_bpt_attr`                                           |       |
| `idc.SetBptCndEx`                         | `idc.set_bpt_cond`                                           |       |
| `idc.SetBptCnd`                           | `idc.set_bpt_cond`                                           |       |
| `idc.AddBptEx`                            | `ida_dbg.add_bpt`                                            |       |
| `idc.AddBpt`                              | `ida_dbg.add_bpt`                                            |       |
| `idc.DelBpt`                              | `ida_dbg.del_bpt`                                            |       |
| `idc.EnableBpt`                           | `ida_dbg.enable_bpt`                                         |       |
| `idc.CheckBpt`                            | `ida_dbg.check_bpt`                                          |       |
| `idc.LoadDebugger`                        | `ida_dbg.load_debugger`                                      |       |
| `idc.StartDebugger`                       | `ida_dbg.start_process`                                      |       |
| `idc.StopDebugger`                        | `ida_dbg.exit_process`                                       |       |
| `idc.PauseProcess`                        | `ida_dbg.suspend_process`                                    |       |
| `idc.GetProcessQty()`                     | `ida_dbg.get_processes().size`                               |       |
| `idc.GetProcessPid(idx)`                  | `ida_dbg.get_processes()[idx].pid`                           |       |
| `idc.GetProcessName(idx)`                 | `ida_dbg.get_processes()[idx].name`                          |       |
| `idc.AttachProcess`                       | `ida_dbg.attach_process`                                     |       |
| `idc.DetachProcess`                       | `ida_dbg.detach_process`                                     |       |
| `idc.GetThreadQty`                        | `ida_dbg.get_thread_qty`                                     |       |
| `idc.GetThreadId`                         | `ida_dbg.getn_thread`                                        |       |
| `idc.GetCurrentThreadId`                  | `ida_dbg.get_current_thread`                                 |       |
| `idc.SelectThread`                        | `ida_dbg.select_thread`                                      |       |
| `idc.SuspendThread`                       | `ida_dbg.suspend_thread`                                     |       |
| `idc.ResumeThread`                        | `ida_dbg.resume_thread`                                      |       |
| `idc.GetFirstModule`                      | `idc.get_first_module`                                       |       |
| `idc.GetNextModule`                       | `idc.get_next_module`                                        |       |
| `idc.GetModuleName`                       | `idc.get_module_name`                                        |       |
| `idc.GetModuleSize`                       | `idc.get_module_size`                                        |       |
| `idc.StepInto`                            | `ida_dbg.step_into`                                          |       |
| `idc.StepOver`                            | `ida_dbg.step_over`                                          |       |
| `idc.RunTo`                               | `ida_dbg.run_to`                                             |       |
| `idc.StepUntilRet`                        | `ida_dbg.step_until_ret`                                     |       |
| `idc.GetDebuggerEvent`                    | `ida_dbg.wait_for_next_event`                                |       |
| `idc.GetProcessState`                     | `ida_dbg.get_process_state`                                  |       |
| `idc.SetDebuggerOptions`                  | `ida_dbg.set_debugger_options`                               |       |
| `idc.SetRemoteDebugger`                   | `ida_dbg.set_remote_debugger`                                |       |
| `idc.GetDebuggerEventCondition`           | `ida_dbg.get_debugger_event_cond`                            |       |
| `idc.SetDebuggerEventCondition`           | `ida_dbg.set_debugger_event_cond`                            |       |
| `idc.GetEventId`                          | `idc.get_event_id`                                           |       |
| `idc.GetEventPid`                         | `idc.get_event_pid`                                          |       |
| `idc.GetEventTid`                         | `idc.get_event_tid`                                          |       |
| `idc.GetEventEa`                          | `idc.get_event_ea`                                           |       |
| `idc.IsEventHandled`                      | `idc.is_event_handled`                                       |       |
| `idc.GetEventModuleName`                  | `idc.get_event_module_name`                                  |       |
| `idc.GetEventModuleBase`                  | `idc.get_event_module_base`                                  |       |
| `idc.GetEventModuleSize`                  | `idc.get_event_module_size`                                  |       |
| `idc.GetEventExitCode`                    | `idc.get_event_exit_code`                                    |       |
| `idc.GetEventInfo`                        | `idc.get_event_info`                                         |       |
| `idc.GetEventBptHardwareEa`               | `idc.get_event_bpt_hea`                                      |       |
| `idc.GetEventExceptionCode`               | `idc.get_event_exc_code`                                     |       |
| `idc.GetEventExceptionEa`                 | `idc.get_event_exc_ea`                                       |       |
| `idc.GetEventExceptionInfo`               | `idc.get_event_exc_info`                                     |       |
| `idc.CanExceptionContinue`                | `idc.can_exc_continue`                                       |       |
| `idc.RefreshDebuggerMemory`               | `ida_dbg.refresh_debugger_memory`                            |       |
| `idc.TakeMemorySnapshot`                  | `ida_segment.take_memory_snapshot`                           |       |
| `idc.EnableTracing`                       | `idc.enable_tracing`                                         |       |
| `idc.GetStepTraceOptions`                 | `ida_dbg.get_step_trace_options`                             |       |
| `idc.SetStepTraceOptions`                 | `ida_dbg.set_step_trace_options`                             |       |
| `idc.DefineException`                     | `ida_dbg.define_exception`                                   |       |
| `idc.BeginTypeUpdating`                   | `ida_typeinf.begin_type_updating`                            |       |
| `idc.EndTypeUpdating`                     | `ida_typeinf.end_type_updating`                              |       |
| `idc.begin_type_updating`                 | `ida_typeinf.begin_type_updating`                            |       |
| `idc.end_type_updating`                   | `ida_typeinf.end_type_updating`                              |       |
| `idc.ValidateNames`                       | `idc.validate_idb_names`                                     |       |
| `idc.SegAlign(ea, alignment)`             | `idc.set_segm_attr(ea, SEGATTR_ALIGN, alignment)`            |       |
| `idc.SegComb(ea, comb)`                   | `idc.set_segm_attr(ea, SEGATTR_COMB, comb)`                  |       |
| `idc.MakeComm(ea, cmt)`                   | `idc.set_cmt(ea, cmt, 0)`                                    |       |
| `idc.MakeRptCmt(ea, cmt)`                 | `idc.set_cmt(ea, cmt, 1)`                                    |       |
| `idc.MakeUnkn`                            | `ida_bytes.del_items`                                        |       |
| `idc.MakeUnknown`                         | `ida_bytes.del_items`                                        |       |
| `idc.LineA(ea, n)`                        | `ida_lines.get_extra_cmt(ea, E_PREV + (n))`                  |       |
| `idc.LineB(ea, n)`                        | `ida_lines.get_extra_cmt(ea, E_NEXT + (n))`                  |       |
| `idc.ExtLinA(ea, n, line)`                | `ida_lines.update_extra_cmt(ea, E_PREV + (n), line)`         |       |
| `idc.ExtLinB(ea, n, line)`                | `ida_lines.update_extra_cmt(ea, E_NEXT + (n), line)`         |       |
| `idc.DelExtLnA(ea, n)`                    | `ida_lines.del_extra_cmt(ea, E_PREV + (n))`                  |       |
| `idc.DelExtLnB(ea, n)`                    | `ida_lines.del_extra_cmt(ea, E_NEXT + (n))`                  |       |
| `idc.SetSpDiff`                           | `ida_frame.add_user_stkpnt`                                  |       |
| `idc.AddUserStkPnt`                       | `ida_frame.add_user_stkpnt`                                  |       |
| `idc.NameEx(From, ea)`                    | `idc.get_name(ea, ida_name.GN_VISIBLE | calc_gtn_flags(From, ea))` |       |
| `idc.GetTrueNameEx(From, ea)`             | `idc.get_name(ea, calc_gtn_flags(From, ea))`                 |       |
| `idc.Message`                             | `ida_kernwin.msg`                                            |       |
| `idc.UMessage`                            | `ida_kernwin.msg`                                            |       |
| `idc.DelSeg`                              | `ida_segment.del_segm`                                       |       |
| `idc.Wait`                                | `ida_auto.auto_wait`                                         |       |
| `idc.LoadTraceFile`                       | `ida_dbg.load_trace_file`                                    |       |
| `idc.SaveTraceFile`                       | `ida_dbg.save_trace_file`                                    |       |
| `idc.CheckTraceFile`                      | `ida_dbg.is_valid_trace_file`                                |       |
| `idc.DiffTraceFile`                       | `ida_dbg.diff_trace_file`                                    |       |
| `idc.SetTraceDesc`                        | `ida_dbg.get_trace_file_desc`                                |       |
| `idc.GetTraceDesc`                        | `ida_dbg.set_trace_file_desc`                                |       |
| `idc.GetMaxTev`                           | `ida_dbg.get_tev_qty`                                        |       |
| `idc.GetTevEa`                            | `ida_dbg.get_tev_ea`                                         |       |
| `idc.GetTevType`                          | `ida_dbg.get_tev_type`                                       |       |
| `idc.GetTevTid`                           | `ida_dbg.get_tev_tid`                                        |       |
| `idc.GetTevRegVal`                        | `ida_dbg.get_tev_reg`                                        |       |
| `idc.GetTevRegMemQty`                     | `ida_dbg.get_tev_mem_qty`                                    |       |
| `idc.GetTevRegMem`                        | `ida_dbg.get_tev_mem`                                        |       |
| `idc.GetTevRegMemEa`                      | `ida_dbg.get_tev_mem_ea`                                     |       |
| `idc.GetTevCallee`                        | `ida_dbg.get_call_tev_callee`                                |       |
| `idc.GetTevReturn`                        | `ida_dbg.get_ret_tev_return`                                 |       |
| `idc.GetBptTevEa`                         | `ida_dbg.get_bpt_tev_ea`                                     |       |
| `idc.ArmForceBLJump`                      | `idc.force_bl_jump`                                          |       |
| `idc.ArmForceBLCall`                      | `idc.force_bl_call`                                          |       |
| `idc.BochsCommand`                        | `idc.send_dbg_command`                                       |       |
| `idc.SendDbgCommand`                      | `idc.send_dbg_command`                                       |       |
| `idc.SendGDBMonitor`                      | `idc.send_dbg_command`                                       |       |
| `idc.WinDbgCommand`                       | `idc.send_dbg_command`                                       |       |
| `idc.SetAppcallOptions(x)`                | `idc.set_inf_attr(INF_APPCALL_OPTIONS, x)`                   |       |
| `idc.GetAppcallOptions()`                 | `idc.get_inf_attr(INF_APPCALL_OPTIONS)`                      |       |
| `idc.AF2_ANORET`                          | `ida_ida.AF_ANORET`                                          |       |
| `idc.AF2_CHKUNI`                          | `ida_ida.AF_CHKUNI`                                          |       |
| `idc.AF2_DATOFF`                          | `ida_ida.AF_DATOFF`                                          |       |
| `idc.AF2_DOCODE`                          | `ida_ida.AF_DOCODE`                                          |       |
| `idc.AF2_DODATA`                          | `ida_ida.AF_DODATA`                                          |       |
| `idc.AF2_FTAIL`                           | `ida_ida.AF_FTAIL`                                           |       |
| `idc.AF2_HFLIRT`                          | `ida_ida.AF_HFLIRT`                                          |       |
| `idc.AF2_JUMPTBL`                         | `ida_ida.AF_JUMPTBL`                                         |       |
| `idc.AF2_PURDAT`                          | `ida_ida.AF_PURDAT`                                          |       |
| `idc.AF2_REGARG`                          | `ida_ida.AF_REGARG`                                          |       |
| `idc.AF2_SIGCMT`                          | `ida_ida.AF_SIGCMT`                                          |       |
| `idc.AF2_SIGMLT`                          | `ida_ida.AF_SIGMLT`                                          |       |
| `idc.AF2_STKARG`                          | `ida_ida.AF_STKARG`                                          |       |
| `idc.AF2_TRFUNC`                          | `ida_ida.AF_TRFUNC`                                          |       |
| `idc.AF2_VERSP`                           | `ida_ida.AF_VERSP`                                           |       |
| `idc.AF_ASCII`                            | `ida_ida.AF_STRLIT`                                          |       |
| `idc.ASCF_AUTO`                           | `ida_ida.STRF_AUTO`                                          |       |
| `idc.ASCF_COMMENT`                        | `ida_ida.STRF_COMMENT`                                       |       |
| `idc.ASCF_GEN`                            | `ida_ida.STRF_GEN`                                           |       |
| `idc.ASCF_SAVECASE`                       | `ida_ida.STRF_SAVECASE`                                      |       |
| `idc.ASCF_SERIAL`                         | `ida_ida.STRF_SERIAL`                                        |       |
| `idc.ASCSTR_C`                            | `ida_nalt.STRTYPE_C`                                         |       |
| `idc.ASCSTR_LEN2`                         | `ida_nalt.STRTYPE_LEN2`                                      |       |
| `idc.ASCSTR_LEN4`                         | `ida_nalt.STRTYPE_LEN4`                                      |       |
| `idc.ASCSTR_PASCAL`                       | `ida_nalt.STRTYPE_PASCAL`                                    |       |
| `idc.ASCSTR_TERMCHR`                      | `ida_nalt.STRTYPE_TERMCHR`                                   |       |
| `idc.ASCSTR_ULEN2`                        | `ida_nalt.STRTYPE_LEN2_16`                                   |       |
| `idc.ASCSTR_ULEN4`                        | `ida_nalt.STRTYPE_LEN4_16`                                   |       |
| `idc.ASCSTR_UNICODE`                      | `ida_nalt.STRTYPE_C_16`                                      |       |
| `idc.DOUNK_SIMPLE`                        | `ida_bytes.DELIT_SIMPLE`                                     |       |
| `idc.DOUNK_EXPAND`                        | `ida_bytes.DELIT_EXPAND`                                     |       |
| `idc.DOUNK_DELNAMES`                      | `ida_bytes.DELIT_DELNAMES`                                   |       |
| `idc.FF_ASCI`                             | `ida_bytes.FF_STRLIT`                                        |       |
| `idc.FF_DWRD`                             | `ida_bytes.FF_DWORD`                                         |       |
| `idc.FF_OWRD`                             | `ida_bytes.FF_OWORD`                                         |       |
| `idc.FF_QWRD`                             | `ida_bytes.FF_QWORD`                                         |       |
| `idc.FF_STRU`                             | `ida_bytes.FF_STRUCT`                                        |       |
| `idc.FF_TBYT`                             | `ida_bytes.FF_TBYTE`                                         |       |
| `idc.FIXUP_BYTE`                          | `ida_fixup.FIXUP_OFF8`                                       |       |
| `idc.FIXUP_CREATED`                       | `ida_fixup.FIXUPF_CREATED`                                   |       |
| `idc.FIXUP_EXTDEF`                        | `ida_fixup.FIXUPF_EXTDEF`                                    |       |
| `idc.FIXUP_REL`                           | `ida_fixup.FIXUPF_REL`                                       |       |
| `idc.FIXUP_UNUSED`                        | `ida_fixup.FIXUPF_UNUSED`                                    |       |
| `idc.GetFlags`                            | `ida_bytes.get_full_flags`                                   |       |
| `idc.ResumeProcess`                       | `idc.resume_process`                                         |       |
| `idc.isEnabled`                           | `ida_bytes.is_mapped`                                        |       |
| `idc.hasValue`                            | `ida_bytes.has_value`                                        |       |
| `idc.isByte`                              | `ida_bytes.is_byte`                                          |       |
| `idc.isWord`                              | `ida_bytes.is_word`                                          |       |
| `idc.isDwrd`                              | `ida_bytes.is_dword`                                         |       |
| `idc.isQwrd`                              | `ida_bytes.is_qword`                                         |       |
| `idc.isOwrd`                              | `ida_bytes.is_oword`                                         |       |
| `idc.isTbyt`                              | `ida_bytes.is_tbyte`                                         |       |
| `idc.isFloat`                             | `ida_bytes.is_float`                                         |       |
| `idc.isDouble`                            | `ida_bytes.is_double`                                        |       |
| `idc.isASCII`                             | `ida_bytes.is_strlit`                                        |       |
| `idc.isStruct`                            | `ida_bytes.is_struct`                                        |       |
| `idc.isAlign`                             | `ida_bytes.is_align`                                         |       |
| `idc.isChar0`                             | `ida_bytes.is_char0`                                         |       |
| `idc.isChar1`                             | `ida_bytes.is_char1`                                         |       |
| `idc.isCode`                              | `ida_bytes.is_code`                                          |       |
| `idc.isData`                              | `ida_bytes.is_data`                                          |       |
| `idc.isDefArg0`                           | `ida_bytes.is_defarg0`                                       |       |
| `idc.isDefArg1`                           | `ida_bytes.is_defarg1`                                       |       |
| `idc.isEnum0`                             | `ida_bytes.is_enum0`                                         |       |
| `idc.isEnum1`                             | `ida_bytes.is_enum1`                                         |       |
| `idc.isFlow`                              | `ida_bytes.is_flow`                                          |       |
| `idc.isHead`                              | `ida_bytes.is_head`                                          |       |
| `idc.isLoaded`                            | `ida_bytes.is_loaded`                                        |       |
| `idc.isOff0`                              | `ida_bytes.is_off0`                                          |       |
| `idc.isOff1`                              | `ida_bytes.is_off1`                                          |       |
| `idc.isPackReal`                          | `ida_bytes.is_pack_real`                                     |       |
| `idc.isSeg0`                              | `ida_bytes.is_seg0`                                          |       |
| `idc.isSeg1`                              | `ida_bytes.is_seg1`                                          |       |
| `idc.isStkvar0`                           | `ida_bytes.is_stkvar0`                                       |       |
| `idc.isStkvar1`                           | `ida_bytes.is_stkvar1`                                       |       |
| `idc.isStroff0`                           | `ida_bytes.is_stroff0`                                       |       |
| `idc.isStroff1`                           | `ida_bytes.is_stroff1`                                       |       |
| `idc.isTail`                              | `ida_bytes.is_tail`                                          |       |
| `idc.isUnknown`                           | `ida_bytes.is_unknown`                                       |       |
| `idc.SEGDEL_KEEP`                         | `ida_segment.SEGMOD_KEEP`                                    |       |
| `idc.SEGDEL_PERM`                         | `ida_segment.SEGMOD_KILL`                                    |       |
| `idc.SEGDEL_SILENT`                       | `ida_segment.SEGMOD_SILENT`                                  |       |
| `idc.SETPROC_ALL`                         | `ida_idp.SETPROC_LOADER_NON_FATAL`                           |       |
| `idc.SETPROC_COMPAT`                      | `ida_idp.SETPROC_IDB`                                        |       |
| `idc.SETPROC_FATAL`                       | `ida_idp.SETPROC_LOADER`                                     |       |
| `idc.INF_CHANGE_COUNTER`                  | `idc.INF_DATABASE_CHANGE_COUNT`                              |       |
| `idc.INF_LOW_OFF`                         | `idc.INF_LOWOFF`                                             |       |
| `idc.INF_HIGH_OFF`                        | `idc.INF_HIGHOFF`                                            |       |
| `idc.INF_START_PRIVRANGE`                 | `idc.INF_PRIVRANGE_START_EA`                                 |       |
| `idc.INF_END_PRIVRANGE`                   | `idc.INF_PRIVRANGE_END_EA`                                   |       |
| `idc.INF_TYPE_XREFS`                      | `idc.INF_TYPE_XREFNUM`                                       |       |
| `idc.INF_REFCMTS`                         | `idc.INF_REFCMTNUM`                                          |       |
| `idc.INF_XREFS`                           | `idc.INF_XREFFLAG`                                           |       |
| `idc.INF_NAMELEN`                         | `idc.INF_MAX_AUTONAME_LEN`                                   |       |
| `idc.INF_SHORT_DN`                        | `idc.INF_SHORT_DEMNAMES`                                     |       |
| `idc.INF_LONG_DN`                         | `idc.INF_LONG_DEMNAMES`                                      |       |
| `idc.INF_CMTFLAG`                         | `idc.INF_CMTFLG`                                             |       |
| `idc.INF_BORDER`                          | `idc.INF_LIMITER`                                            |       |
| `idc.INF_BINPREF`                         | `idc.INF_BIN_PREFIX_SIZE`                                    |       |
| `idc.INF_COMPILER`                        | `idc.INF_CC_ID`                                              |       |
| `idc.INF_MODEL`                           | `idc.INF_CC_CM`                                              |       |
| `idc.INF_SIZEOF_INT`                      | `idc.INF_CC_SIZE_I`                                          |       |
| `idc.INF_SIZEOF_BOOL`                     | `idc.INF_CC_SIZE_B`                                          |       |
| `idc.INF_SIZEOF_ENUM`                     | `idc.INF_CC_SIZE_E`                                          |       |
| `idc.INF_SIZEOF_ALGN`                     | `idc.INF_CC_DEFALIGN`                                        |       |
| `idc.INF_SIZEOF_SHORT`                    | `idc.INF_CC_SIZE_S`                                          |       |
| `idc.INF_SIZEOF_LONG`                     | `idc.INF_CC_SIZE_L`                                          |       |
| `idc.INF_SIZEOF_LLONG`                    | `idc.INF_CC_SIZE_LL`                                         |       |
| `idc.INF_SIZEOF_LDBL`                     | `idc.INF_CC_SIZE_LDBL`                                       |       |
| `idc.REF_VHIGH`                           | `ida_nalt.V695_REF_VHIGH`                                    |       |
| `idc.REF_VLOW`                            | `ida_nalt.V695_REF_VLOW`                                     |       |
| `idc.UTP_STRUCT`                          | `ida_typeinf.UTP_STRUCT`                                     |       |
| `idc.UTP_ENUM`                            | `ida_typeinf.UTP_ENUM`                                       |       |
| `idc.GetOpnd`                             | `idc.print_operand`                                          |       |
| `idc.patch_long`                          | `ida_bytes.patch_dword`                                      |       |
| `idc.python_on()`                         | `ida_loader.load_and_run_plugin("idapython", 3)`             |       |
| `idc.RunPythonStatement`                  | `idc.exec_python`                                            |       |
| `idc.GetManyBytes`                        | `idc.get_bytes`                                              |       |
| `idc.GetString`                           | `idc.get_strlit_contents`                                    |       |
| `idc.ClearTraceFile`                      | `idc.clear_trace`                                            |       |
| `idc.FindBinary`                          | `idc.find_binary`                                            |       |
| `idc.FindText`                            | `idc.find_text`                                              |       |
| `idc.NextHead`                            | `idc.next_head`                                              |       |
| `idc.PrevHead`                            | `idc.prev_head`                                              |       |
| `idc.ProcessUiAction`                     | `ida_kernwin.process_ui_action`                              |       |
| `idc.SaveBase`                            | `idc.save_database`                                          |       |
| `idc.GetProcessorName()`                  | `ida_ida.inf_get_procname()`                                 |       |
| `idc.SegStart`                            | `idc.get_segm_start`                                         |       |
| `idc.SegEnd`                              | `idc.get_segm_end`                                           |       |
| `idc.SetSegmentType`                      | `idc.set_segm_type`                                          |       |



