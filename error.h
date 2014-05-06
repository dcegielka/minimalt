#ifndef ERROR_H
#define ERROR_H

#define returnerr(e) do{ const char *err_ = (e); if (err_) { return err_; } }while(0)

#define error const char *__attribute__((warn_unused_result))

void fatal(const char *err);

#endif
