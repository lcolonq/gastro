;;; convert --- convert obj file -*- lexical-binding: t; -*-
;;; Commentary:
;;; Code:

(require 's)
(require 'ht)
(require 'wasp-utils)

(defun g/fix-from-double (x)
  "Convert X to a fixed-point (string)."
  (format "%d" (truncate (* x (ash 1 16)))))

(defun g/convert-obj (p)
  "Convert OBJ file at P."
  (-let*
    ( ((sv si) (s-split "\n\n" (w/slurp p)))
      (verts (--mapcat (-map #'string-to-number (cdr (s-split " " it))) (s-lines sv)))
      (idxs (--mapcat (-map (lambda (s) (- (string-to-number s) 1)) (cdr (s-split " " it))) (s-lines si))))
    (with-temp-buffer
      (insert "static void draw_model(gastro_ctx *ctx, gastro_program *p) {\n")
      (insert "    static gastro_fix64 verts[] = {\n");
      (insert (s-join ",\n" (-map #'g/fix-from-double verts))) (insert "\n")
      (insert "    };\n");
      (insert "    static i64 idxs[] = {\n");
      (insert (s-join ",\n" (--map (format "%s" it) idxs))) (insert "\n")
      (insert "    };\n");
      (insert (format "    gastro_render_triangles(ctx, p, verts, 3, idxs, %s);\n" (/ (length idxs) 3)))
      (insert "};\n")
      (w/spit "converted_model.h" (buffer-string)))))

(defun g/convert-obj2 (p)
  (-let*
    ( ((sv st sn si) (s-split "\n\n" (w/slurp p)))
      (cache (ht-create))
      (vs nil)
      (verts (seq-into (--map (-map #'string-to-number (cdr (s-split " " it))) (s-lines sv)) 'vector))
      (texs (seq-into (--map (-map #'string-to-number (cdr (s-split " " it))) (s-lines st)) 'vector))
      (norms (seq-into (--map (-map #'string-to-number (cdr (s-split " " it))) (s-lines sn)) 'vector))
      (addv
        (lambda (vstr)
          (if-let* ((idx (ht-get cache vstr)))
            idx
            (-let ( (idx (length vs))
                    ((iv it in) (-map #'string-to-number (s-split "/" vstr))))
              (ht-set! cache vstr idx)
              (push (-concat (seq-elt verts (- iv 1)) (seq-elt texs (- it 1)) (seq-elt norms (- in 1))) vs)
              idx))))
      (idxs
        (--mapcat
          (-let [(p0 p1 p2) (cdr (s-split " " it))]
            (list
              (funcall addv p0)
              (funcall addv p1)
              (funcall addv p2)))
          (s-lines si)))
      (allverts (apply #'-concat (reverse vs))))
    (with-temp-buffer
      (insert "static void draw_model(gastro_ctx *ctx, gastro_program *p) {\n")
      (insert "    static gastro_fix64 verts[] = {\n");
      (insert (s-join ",\n" (-map #'g/fix-from-double allverts))) (insert "\n")
      (insert "    };\n");
      (insert "    static i64 idxs[] = {\n");
      (insert (s-join ",\n" (--map (format "%s" it) idxs))) (insert "\n")
      (insert "    };\n");
      (insert (format "    gastro_render_triangles(ctx, p, verts, 8, idxs, %s);\n" (/ (length idxs) 3)))
      (insert "};\n")
      (w/spit "converted_model.h" (buffer-string)))))

(g/convert-obj2 "teapot2.obj")

(provide 'convert)
;;; convert.el ends here
