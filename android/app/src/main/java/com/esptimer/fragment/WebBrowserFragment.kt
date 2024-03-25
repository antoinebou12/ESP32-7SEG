package com.esptimer.fragment

import android.annotation.SuppressLint
import android.os.Bundle
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.webkit.WebView
import android.webkit.WebViewClient
import androidx.fragment.app.Fragment
import com.esptimer.R

class WebBrowserFragment : Fragment() {
    private var webView: WebView? = null
    private var url: String? = "http://192.168.0.38/"

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        // Retrieve the URL passed to this Fragment
        url = arguments?.getString("url")
    }

    @SuppressLint("SetJavaScriptEnabled")
    override fun onCreateView(
        inflater: LayoutInflater,
        container: ViewGroup?,
        savedInstanceState: Bundle?
    ): View? {
        // Inflate the layout for this fragment
        val view = inflater.inflate(R.layout.fragment_web_browser, container, false)
        webView = view.findViewById(R.id.webview) // Corrected to match the XML layout ID
        webView?.settings?.javaScriptEnabled = true // Enable JavaScript

        // Setup WebViewClient to handle loading URLs inside the WebView
        webView?.webViewClient = WebViewClient()

        // Load the URL passed to this Fragment
        webView?.loadUrl(url ?: "http://192.168.0.38/") // Default URL if none provided

        return view
    }


    companion object {
        fun newInstance(url: String?): WebBrowserFragment {
            val fragment = WebBrowserFragment()
            val args = Bundle().apply {
                putString("url", url)
            }
            fragment.arguments = args
            return fragment
        }
    }

    override fun onDestroyView() {
        super.onDestroyView()
        webView?.destroy()
        webView = null // Release the reference to the WebView to avoid memory leaks
    }
}
