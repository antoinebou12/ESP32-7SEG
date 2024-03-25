package com.esptimer.adapter

import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView
import com.esptimer.R

class SettingsAdapter(private val settingsList: List<SettingItem>) :
    RecyclerView.Adapter<SettingsAdapter.SettingsViewHolder>() {

    class SettingsViewHolder(view: View) : RecyclerView.ViewHolder(view) {
        val titleTextView: TextView = view.findViewById(R.id.settingTitleTextView)
        val valueTextView: TextView = view.findViewById(R.id.settingValueTextView)
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): SettingsViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.item_setting, parent, false)
        return SettingsViewHolder(view)
    }

    override fun onBindViewHolder(holder: SettingsViewHolder, position: Int) {
        val item = settingsList[position]
        holder.titleTextView.text = item.title
        holder.valueTextView.text = item.value

        if (item.action != null) {
            holder.itemView.setOnClickListener {
                item.action.invoke()
            }
        } else {
            holder.itemView.setOnClickListener(null)
        }
    }

    override fun getItemCount() = settingsList.size
}

data class SettingItem(val title: String, val value: String, val action: (() -> Unit)? = null)
