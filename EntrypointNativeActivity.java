// this is just an example
// you will most likely need more stuff to make it work

package com.beardsvibe;

import android.content.Intent;
import android.net.Uri;

public class EntrypointNativeActivity extends android.app.NativeActivity {
    public static void openURL(String url) {
        activity.startActivity(new Intent(Intent.ACTION_VIEW, Uri.parse(url)));
    }
}
