/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

/*
 * Test.java
 *
 * Created on Jan 8, 2011, 10:45:00 AM
 */

package edu.stanford.atom.sti.application.pdmonitor;

import edu.stanford.atom.sti.client.gui.application.STIApplicationPanel;
import edu.stanford.atom.sti.corba.Types.*;
import edu.stanford.atom.sti.client.comm.bl.TChannelDecode;
import edu.stanford.atom.sti.client.gui.CellEditTable.*;
import java.util.*;
import java.awt.Color;

import edu.stanford.atom.sti.client.gui.ColorTable.*;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.TableModel;


/**
 *
 * @author Jason
 */
public class PDMonitorApp extends STIApplicationPanel implements TableModelListener {

    //private ArrayList<TChannel> inputChannels;
    private HashMap<Short, String> powers = new HashMap<Short, String>();
    private HashMap<Short, ArrayList> channelPosition;
    private Map<Short,PhotoDetector> photoDetectorMap;
    private Collection <PhotoDetector> photoDetectors;

    //Device has not been set by the time the constructor executes. Use this bool
    //with tabIsVisible to fill the photoDetectorMap
    private Boolean hasBeenInitialized = false;

    //displacement of labels and setpoints from the data
    int setpointRowOffset = -1;
    int labelRowOffset = -2;

    /** Creates new form Test */
    public PDMonitorApp() {
        initComponents();

        ColorTableModel ctm =
                new ColorTableModel(powerTable,
                javax.swing.UIManager.getDefaults().getColor("Button.background"));
        powerTable.setModel(ctm);
        powerTable.setCellEditorModel(new CellEditorModel());

    }

    public void handleDeviceEvent(edu.stanford.atom.sti.client.comm.bl.device.DeviceEvent evt) {
        
    }
    
    public void tabIsHidden() {
    }
    public void tabIsVisible() {

        //Initialized photoDetectorMap once the device has been set
        if (!hasBeenInitialized) {
            //Get a list of the channels.
        TChannel[] channels = getDevice().getChannels();
//        inputChannels = new ArrayList<TChannel>();
//        photoDetectorMap = new HashMap<Short,PhotoDetector>();
        photoDetectors = new ArrayList<PhotoDetector>();
        
            for (TChannel channel : channels)
            {
                TChannelDecode decoded = new TChannelDecode(channel);
                // load only channels that are inputs. Channel 0 is for the call
                // function and is not a photodetector
                if (decoded.IOType().equals("Input")&& channel.channel != 0)
                {
                    photoDetectors.add(new PhotoDetector(channel.channel));
                }
            }

            for (PhotoDetector photoDetector : photoDetectors)
            {
                //Get position of PD in table
                TDataMixed positionVector = call("getChannelLayout", photoDetector.deviceChannel);
                if (positionVector.discriminator() != TData.DataVector) {
                    System.out.println("Expecting a vector from getChannelLayout in PDMonitorApp");
                    return;
                }
                TDataMixed[] position = call("getChannelLayout", photoDetector.deviceChannel).vector();

                //Get label of PD
                TDataMixed label = call("getChannelLabel", photoDetector.deviceChannel);
                if (label.discriminator() != TData.DataString)
                {
                    System.out.println("Expecting a string from getChannelLabel in PDMonitorApp");
                    return;
                }
                
                //Get setpoint of PD
                TDataMixed setpoint = call("getChannelSetpoint",photoDetector.deviceChannel);
                if (setpoint.discriminator() != TData.DataDouble){
                    System.out.println("Expecting a double from getChannelLabel in PDMonitorApp");
                    return;
                }

                if (position.length == 2) {
                    photoDetector.columnPosition = (int) position[0].longVal();
                    photoDetector.rowPosition = (int) position[1].longVal();

                    photoDetector.label = label.stringVal();

                    photoDetector.setPoint = setpoint.doubleVal();
                } else {
                    System.out.println("Expect a vector of length 2 from getChannelLayout in PDMonitorApp");
                    return;
                }

            }

            initPowerTable();
            updatePowerTable();
            hasBeenInitialized = true;
        }
    }


    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        pdMonitorLabel = new javax.swing.JLabel();
        pdMonitorSplitPane = new javax.swing.JSplitPane();
        readoutScrollPane = new javax.swing.JScrollPane();
        readoutPanel = new javax.swing.JPanel();
        readPowerButtons = new javax.swing.JButton();
        colorTolerance = new javax.swing.JTextField();
        jLabel1 = new javax.swing.JLabel();
        saveSetpointsButton = new javax.swing.JButton();
        controlScrollPane = new javax.swing.JScrollPane();
        controlPanel = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        powerTable = new edu.stanford.atom.sti.client.gui.CellEditTable.CellEditTable();

        pdMonitorLabel.setFont(new java.awt.Font("Tahoma", 0, 18));
        pdMonitorLabel.setText("PD Monitor App GUI");

        pdMonitorSplitPane.setDividerLocation(250);

        readoutPanel.setPreferredSize(new java.awt.Dimension(250, 450));

        readPowerButtons.setText("Read");
        readPowerButtons.addMouseListener(new java.awt.event.MouseAdapter() {
            public void mouseClicked(java.awt.event.MouseEvent evt) {
                readPowerButtonsMouseClicked(evt);
            }
        });
        readPowerButtons.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                readPowerButtonsActionPerformed(evt);
            }
        });

        colorTolerance.setHorizontalAlignment(javax.swing.JTextField.RIGHT);
        colorTolerance.setText("10");
        colorTolerance.setPreferredSize(new java.awt.Dimension(40, 20));
        colorTolerance.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                colorToleranceActionPerformed(evt);
            }
        });

        jLabel1.setText("Tolerance (%)");

        saveSetpointsButton.setText("Save Setpoints");
        saveSetpointsButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                saveSetpointsButtonActionPerformed(evt);
            }
        });

        javax.swing.GroupLayout readoutPanelLayout = new javax.swing.GroupLayout(readoutPanel);
        readoutPanel.setLayout(readoutPanelLayout);
        readoutPanelLayout.setHorizontalGroup(
            readoutPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(readoutPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(readoutPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(readPowerButtons)
                    .addComponent(jLabel1)
                    .addComponent(colorTolerance, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(saveSetpointsButton))
                .addContainerGap(135, Short.MAX_VALUE))
        );
        readoutPanelLayout.setVerticalGroup(
            readoutPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(readoutPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(readPowerButtons)
                .addGap(18, 18, 18)
                .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(colorTolerance, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(18, 18, 18)
                .addComponent(saveSetpointsButton)
                .addContainerGap(317, Short.MAX_VALUE))
        );

        readoutScrollPane.setViewportView(readoutPanel);

        pdMonitorSplitPane.setLeftComponent(readoutScrollPane);

        controlPanel.setPreferredSize(new java.awt.Dimension(400, 300));

        powerTable.setBackground(javax.swing.UIManager.getDefaults().getColor("Button.background"));
        powerTable.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {

            },
            new String [] {

            }
        ));
        powerTable.setFont(new java.awt.Font("Tahoma", 1, 14)); // NOI18N
        powerTable.setRowSelectionAllowed(false);
        powerTable.setShowHorizontalLines(false);
        powerTable.setShowVerticalLines(false);
        powerTable.setTableHeader(null);
        jScrollPane1.setViewportView(powerTable);

        javax.swing.GroupLayout controlPanelLayout = new javax.swing.GroupLayout(controlPanel);
        controlPanel.setLayout(controlPanelLayout);
        controlPanelLayout.setHorizontalGroup(
            controlPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(controlPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 383, Short.MAX_VALUE)
                .addContainerGap())
        );
        controlPanelLayout.setVerticalGroup(
            controlPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(controlPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 428, Short.MAX_VALUE)
                .addContainerGap())
        );

        controlScrollPane.setViewportView(controlPanel);

        pdMonitorSplitPane.setRightComponent(controlScrollPane);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(this);
        this.setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(pdMonitorLabel)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(pdMonitorSplitPane, javax.swing.GroupLayout.DEFAULT_SIZE, 512, Short.MAX_VALUE)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(pdMonitorLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(pdMonitorSplitPane, javax.swing.GroupLayout.DEFAULT_SIZE, 319, Short.MAX_VALUE)
                .addContainerGap())
        );
    }// </editor-fold>//GEN-END:initComponents


    private void readPowerButtonsMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_readPowerButtonsMouseClicked
        // TODO add your handling code here:
    }//GEN-LAST:event_readPowerButtonsMouseClicked

    private void readPowerButtonsActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_readPowerButtonsActionPerformed


        for (PhotoDetector photoDetector : photoDetectors)
        {
            TDataMixed power = call("readChannel",photoDetector.deviceChannel,0);
            //powers.put(photoDetector.deviceChannel, power.stringVal());
            //PhotoDetector photoDetector = photoDetectorMap.get(item.channel);
            photoDetector.power = power.doubleVal();
        }

        updatePowerTable();

    }//GEN-LAST:event_readPowerButtonsActionPerformed

    private void colorToleranceActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_colorToleranceActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event_colorToleranceActionPerformed

    private void saveSetpointsButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event_saveSetpointsButtonActionPerformed

        Vector<Object> tmp = new Vector<Object>();
        Vector<Vector<Object>> args = new Vector<Vector<Object>>();


        for (PhotoDetector photoDetector : photoDetectors)
        {
            tmp.add(photoDetector.deviceChannel);
            tmp.add(photoDetector.setPoint);
            args.add(new Vector<Object>(tmp));
            tmp.clear();
        }

        TDataMixed successData = call("writeSetpoints", args);
        Boolean success = successData.booleanVal();
        if (!success)
            System.out.println("Error in writing setpoints");

    }//GEN-LAST:event_saveSetpointsButtonActionPerformed

    private void initPowerTable() {

        //powerTable.setCellEditor(new NumberEditor());
        
        ColorTableModel powerModel = (ColorTableModel) ((CellEditTable) powerTable).getModel();

        SortedSet rowSet = new TreeSet();
        SortedSet colSet = new TreeSet();

        for (PhotoDetector photoDetector : photoDetectors)
        {
            rowSet.add(photoDetector.rowPosition);
            colSet.add(photoDetector.columnPosition);
        }

        //The labels and setpoints need an extra row before each row with PD's in it
        Iterator it = rowSet.iterator();
        Integer lastRow = -1;
        int rowsAdded = 0;
        SortedMap rowShiftMap = new TreeMap();

        while(it.hasNext())
        {
            Integer thisRow = (Integer) it.next();
            if (thisRow == lastRow + 1)
            {
                rowsAdded += 2;
            } else if (thisRow == lastRow + 2)
            {
                rowsAdded++;
            }
            rowShiftMap.put(thisRow, thisRow + rowsAdded);

            lastRow = thisRow;
        }

        //Shift rows to make room for labels
        for (PhotoDetector photoDetector : photoDetectors)
            photoDetector.rowPosition = (Integer) rowShiftMap.get(photoDetector.rowPosition);


        for (int i = 0; i <= (Integer) rowShiftMap.get(rowShiftMap.lastKey()); i++)
            powerModel.addRow(new Object[] {});

        for (int i = 0; i <= ((Integer) colSet.last()) + 1; i++)
            powerModel.addColumn("");

        //Insert labels and setpoints
        for (PhotoDetector photoDetector : photoDetectors) {
            powerTable.getCellEditorModel().addEditorForCell(photoDetector.rowPosition - 1,
                    photoDetector.columnPosition, new NumberEditor());
        }
        powerModel = (ColorTableModel) powerTable.getModel();
        
        for (PhotoDetector photoDetector : photoDetectors)
        {
            powerModel.setValueAt(photoDetector.label,
                    photoDetector.rowPosition + labelRowOffset, photoDetector.columnPosition);
            powerModel.setValueAt(photoDetector.setPoint,
                    photoDetector.rowPosition + setpointRowOffset, photoDetector.columnPosition,
                    Color.white, Boolean.TRUE);
            //does this have to be a new Double?
        }

        powerModel.addTableModelListener(this);
        
    }

    private void updatePowerTable() {

//        Color highPower = Color.green;
//        Color lowPower = Color.red;
//        Color powerStatus = highPower;

        double okPower = 0.33;  //Hue: green
        double lowPower = 0.0;    //Hue: red
        double powerStatus = okPower;
        Object myCell;

        ColorTableModel powerModel = (ColorTableModel) powerTable.getModel();
        for (PhotoDetector photoDetector : photoDetectors)
        {
//            if(photoDetector.power < photoDetector.setPoint)
//                powerStatus = lowPower;
//            else
//                powerStatus = highPower;

            double fracPower = (photoDetector.setPoint - photoDetector.power) /
                    photoDetector.setPoint;

            double tolerance = Double.parseDouble(colorTolerance.getText())/100;

            if (fracPower < 0)
                powerStatus = okPower;
            else if (fracPower > tolerance)
                powerStatus = lowPower;
            else {
                powerStatus = (tolerance - fracPower)/tolerance * (okPower - lowPower);
            }


            powerModel.setValueAt(photoDetector.power, photoDetector.rowPosition,
                    photoDetector.columnPosition, 
                    Color.getHSBColor((float) powerStatus, (float) 1, (float) 1));
        }
        
    }

    public void tableChanged(TableModelEvent e) {
        //Assums only once cell has changed
        int row = e.getFirstRow();
        int column = e.getColumn();
        ColorTableModel model = (ColorTableModel)e.getSource();
        Object data = model.getValueAt(row, column);

        if(data.getClass() != Double.class && data.getClass() != Long.class)
            return;
        //find the cell that must be reset. It will be a setpoint
        for (PhotoDetector photoDetector : photoDetectors)
        {
            if (row == photoDetector.rowPosition + setpointRowOffset &&
                    column == photoDetector.columnPosition)
            {
                if (data.getClass() == Double.class)
                    photoDetector.setPoint = (Double) data;
                else // class is Long. This is ridiculous.
                    photoDetector.setPoint = (double) ((Long) data).intValue();

            }
        }
    }


    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JTextField colorTolerance;
    private javax.swing.JPanel controlPanel;
    private javax.swing.JScrollPane controlScrollPane;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JLabel pdMonitorLabel;
    private javax.swing.JSplitPane pdMonitorSplitPane;
    private edu.stanford.atom.sti.client.gui.CellEditTable.CellEditTable powerTable;
    private javax.swing.JButton readPowerButtons;
    private javax.swing.JPanel readoutPanel;
    private javax.swing.JScrollPane readoutScrollPane;
    private javax.swing.JButton saveSetpointsButton;
    // End of variables declaration//GEN-END:variables

}
