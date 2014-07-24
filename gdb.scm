(use-modules (gdb))

(define (make-jsid-predicate jsid string)
  (lambda (breakpoint)
    (let* ((id (parse-and-eval jsid))
           (pp (value-print id)))
      (and (string? pp)
           (string? string)
           (string-contains jsid string)))))

(define (break-on-jsid location jsid string)
  "Break on location when jsid contains string."
  (let ((bp (make-breakpoint location)))
    (register-breakpoint! bp)
    (set-breakpoint-stop! bp (make-jsid-predicate jsid string))))

(register-command!
 (make-command "break-on-jsid"
               #:command-class COMMAND_BREAKPOINTS
               #:invoke (lambda (self args from-tty)
                          (apply break-on-jsid
                                 (string->argv args)))))
