/* At present,  these are the only two selection types supported.
Then again,  the other types are all deprecated anyway. */

#define CLIPTYPE_CLIPBOARD          0
#define CLIPTYPE_PRIMARY            1

int copy_text_to_clipboard( const char *text, const int clip_type);
char *get_x_selection( const int clip_type);
