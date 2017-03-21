#ifndef U_DUTY
#define U_DUTY

#include <QString>

bool try_str_to_int(QString st, int& int_var) {
    QString txt = st.trimmed();
    int txt_val = txt.toInt();
    bool res = (txt_val != 0) or ((txt_val == 0) and (txt == "0"));
    if (res==true) {
        int_var = txt_val;
    }
    return res;
}

#endif // U_DUTY

