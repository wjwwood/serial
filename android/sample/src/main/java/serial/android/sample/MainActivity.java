package serial.android.sample;

import android.content.Context;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.support.design.widget.FloatingActionButton;
import android.support.design.widget.Snackbar;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.view.View;
import android.view.Menu;
import android.view.MenuItem;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

import java.io.IOException;

import serial.PortInfo;
import serial.Serial;
import serial.SerialIOException;

public class MainActivity extends AppCompatActivity {

    private Spinner mSpinnerPorts;
    private boolean mHexMode;
    private TextView mTextAvailable;
    private TextView mTextOutput;
    private TextView mTextInput;
    private Button mButtonSend;
    private Button mButtonRead;
    private ToggleButton mButtonConnectDisconnect;
    private Serial mPort;

    private Handler mHandler = new Handler();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        Toolbar toolbar = (Toolbar) findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);

        mSpinnerPorts = (Spinner) findViewById(R.id.spinnerPorts);
        mTextOutput = (TextView) findViewById(R.id.dataReceived);
        mTextInput = (TextView) findViewById(R.id.dataToSend);
        mTextAvailable = (TextView) findViewById(R.id.lblAvailable);

        mButtonSend = (Button) findViewById(R.id.btnSend);
        mButtonRead = (Button) findViewById(R.id.btnRead);
        mButtonConnectDisconnect = (ToggleButton) findViewById(R.id.tbConnect);
    }

    @Override
    protected void onStart() {
        super.onStart();
        loadPorts();
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        // Inflate the menu; this adds items to the action bar if it is present.
        getMenuInflater().inflate(R.menu.menu_main, menu);
        return true;
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        MenuItem refreshItem = menu.findItem(R.id.action_reload);
        refreshItem.setEnabled(mPort == null);
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle action bar item clicks here. The action bar will
        // automatically handle clicks on the Home/Up button, so long
        // as you specify a parent activity in AndroidManifest.xml.
        int id = item.getItemId();

        boolean handled = true;
        switch (id) {
            case R.id.action_reload:
                loadPorts();
                break;
            case R.id.action_hex:
                item.setChecked(!item.isChecked());
                mHexMode = item.isChecked();
                clearInput();
                clearOutput();
                Toast.makeText(MainActivity.this, "Not implemented.", Toast.LENGTH_SHORT).show();
                break;
            default:
                handled = false;
                break;
        }


        return handled || super.onOptionsItemSelected(item);
    }

    private void clearOutput() {
        mTextOutput.setText("");
    }

    private void clearInput() {
        mTextInput.setText("");
    }

    private void loadPorts() {
        PortInfo[] ports = Serial.listPorts();
        PortAdapter adapter = new PortAdapter(getApplicationContext(), ports);
        mSpinnerPorts.setAdapter(adapter);
        mButtonConnectDisconnect.setChecked(false);
        mButtonConnectDisconnect.setEnabled(!adapter.isEmpty());
    }

    public void onConnectDisconnectClick(View view) {
        if (mPort != null) {
            try {
                mPort.close();
            } catch (IOException e) {
                // Never get here
            }
            mSpinnerPorts.setEnabled(true);
            mButtonRead.setEnabled(false);
            mButtonSend.setEnabled(false);
            mTextInput.setEnabled(false);
            mPort = null;
        } else {
            clearInput();
            clearOutput();
            PortInfo portInfo = (PortInfo) mSpinnerPorts.getSelectedItem();
            Serial port;
            try {
                port = new Serial.Builder(portInfo.port, Serial.BAUDRATE_115200).create();
                if (port.isValid() && port.isOpen()) {
                    mPort = port;
                    mSpinnerPorts.setEnabled(false);
                    mButtonRead.setEnabled(true);
                    mButtonSend.setEnabled(true);
                    mTextInput.setEnabled(true);
                    checkAvailableData();
                }
            } catch (SerialIOException e) {
                Toast.makeText(MainActivity.this, getString(R.string.toast_connect_failed, portInfo.port), Toast.LENGTH_SHORT).show();
                mTextOutput.setText(e.getMessage());
                mButtonConnectDisconnect.setChecked(false);
            }
        }
    }

    public void onSendClick(View view) {
        if (mPort != null) {
            String text = mTextInput.getText().toString();
            try {
                int bytesWritten = mPort.write(text);
                updateAvailableData();
                clearInput();
                Toast.makeText(MainActivity.this, getString(R.string.toast_send_success, bytesWritten, mPort.getPort()), Toast.LENGTH_SHORT).show();
            } catch (SerialIOException e) {
                Toast.makeText(MainActivity.this, getString(R.string.toast_send_failed, mPort.getPort()), Toast.LENGTH_SHORT).show();
                mTextOutput.setText(e.getMessage());
            }
        }
    }

    public void onReadClick(View view) {
        if (mPort != null) {
            String[] lines;
            try {
                clearOutput();
                byte[] data = mPort.read();
                String s = new String(data);
                mTextOutput.append(s);
                Toast.makeText(MainActivity.this, getString(R.string.toast_read_success, data.length, mPort.getPort()), Toast.LENGTH_SHORT).show();
                updateAvailableData();
            } catch (SerialIOException e) {
                Toast.makeText(MainActivity.this, getString(R.string.toast_send_failed, mPort.getPort()), Toast.LENGTH_SHORT).show();
                mTextOutput.setText(e.getMessage());
            }
        }
    }

    private void updateAvailableData() {
        updateAvailableData(100L);
    }

    private void updateAvailableData(long timeout) {
        if (timeout <= 0) {
            checkAvailableData();
        } else {
            mHandler.postDelayed(new Runnable() {
                @Override
                public void run() {
                    checkAvailableData();
                }
            }, timeout);
        }
    }

    private void checkAvailableData() {
        try {
            mTextAvailable.setText(getString(R.string.hint_available_bytes, mPort.available()));
        } catch (SerialIOException e) {
            mTextAvailable.setText(R.string.hint_unknown_bytes);
            e.printStackTrace();
        }
    }

    private static class PortAdapter extends ArrayAdapter<PortInfo> {

        public PortAdapter(Context context, PortInfo[] ports) {
            super(context, android.R.layout.simple_spinner_item, android.R.id.text1, ports);
            setDropDownViewResource(android.R.layout.simple_list_item_2);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View view = super.getView(position, convertView, parent);
            if (view != convertView) {
                // We created a new view.
                TextView textView = (TextView)view;
                textView.setTextColor(Color.BLACK);
            }
            return view;
        }

        @Override
        public View getDropDownView(int position, View convertView, ViewGroup parent) {
            View view = super.getDropDownView(position, convertView, parent);
            if (view != convertView) {
                // We created a new view.
                TextView descriptionView = (TextView)view.findViewById(android.R.id.text1);
                TextView portView = (TextView)view.findViewById(android.R.id.text2);
                PortInfo portInfo = getItem(position);
                descriptionView.setText(portInfo.description);
                descriptionView.setTextColor(Color.BLACK);
                portView.setText(portInfo.port);
                portView.setTextColor(Color.BLACK);
            }
            return view;
        }
    }
}
