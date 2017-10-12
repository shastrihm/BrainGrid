package edu.uwb.braingrid.workbench.provvisualizer.controller;

import edu.uwb.braingrid.workbench.provvisualizer.ProvVisGlobal;
import edu.uwb.braingrid.workbench.provvisualizer.factory.EdgeFactory;
import edu.uwb.braingrid.workbench.provvisualizer.factory.NodeFactory;
import edu.uwb.braingrid.workbench.provvisualizer.model.ActivityNode;
import edu.uwb.braingrid.workbench.provvisualizer.model.Edge;
import edu.uwb.braingrid.workbench.provvisualizer.model.Graph;
import edu.uwb.braingrid.workbench.provvisualizer.model.Node;
import edu.uwb.braingrid.workbench.provvisualizer.view.VisCanvas;
import javafx.animation.AnimationTimer;
import javafx.beans.value.ChangeListener;
import javafx.beans.value.ObservableValue;
import javafx.event.EventHandler;
import javafx.fxml.FXML;
import javafx.scene.canvas.GraphicsContext;
import javafx.scene.control.Slider;
import javafx.scene.input.MouseButton;
import javafx.scene.input.MouseEvent;
import javafx.scene.input.ScrollEvent;
import javafx.scene.layout.AnchorPane;
import org.apache.jena.rdf.model.Model;
import org.apache.jena.rdf.model.Statement;
import org.apache.jena.rdf.model.StmtIterator;
import org.apache.jena.riot.RDFDataMgr;
import org.controlsfx.control.ToggleSwitch;

public class ProvenanceVisualizerController {
    private Graph dataProvGraph ;

    private GraphicsContext gc ;
    private Model provModel ;
    private AnimationTimer timer;
    private double zoomRatio = 1;
    private Node draggedNode ;
    private double zoomSpeed=.2;

    private double[] pressedXY ;

    private double[] displayWindowLocation = new double[]{ 0, 0 };
    private double[] displayWindowSize = new double[]{ 10000, 10000 };

    private double[] displayWindowLocationTmp ;

    @FXML
    private VisCanvas visCanvas;
    @FXML
    private AnchorPane canvasPane;
    @FXML
    private Slider adjustForceSlider;
    @FXML
    private ToggleSwitch stopForces;
    @FXML
    private ToggleSwitch showNodeIds;
    @FXML
    private ToggleSwitch showRelationships;

    /**
     * Initializes the controller.
     */
    @FXML
    public void initialize(){
        dataProvGraph = new Graph();
        dataProvGraph.setC3(adjustForceSlider.getValue() * 1500);
        gc = visCanvas.getGraphicsContext2D();

        initNodeEdge(System.getProperty("user.dir") + "/projects/haha/provenance/haha.ttl");

        //out of memory at iteration# 2718837
        //initNodeEdge("C:/Users/Choi/Desktop/SugarScape_XS.ttl");

        // Bind canvas size to stack pane size.
        visCanvas.widthProperty().bind(canvasPane.widthProperty());
        visCanvas.heightProperty().bind(canvasPane.heightProperty());

        initMouseEvents();
        initGUIEvents();


        timer = new AnimationTimer() {
            @Override
            public void handle(long now) {
                if(!stopForces.isSelected()) {
                    dataProvGraph.moveNodes(draggedNode);
                }
                dataProvGraph.drawOnCanvas(visCanvas, displayWindowLocation, displayWindowSize, zoomRatio);
            }
        };

        timer.start();
    }

    private void initGUIEvents(){
        adjustForceSlider.valueProperty().addListener(new ChangeListener<Number>() {
            public void changed(ObservableValue<? extends Number> ov, Number old_val, Number new_val) {
                dataProvGraph.setC3(new_val.doubleValue() * 1500);
            }
        });

        showNodeIds.selectedProperty().addListener(new ChangeListener<Boolean>() {
            public void changed(ObservableValue<? extends Boolean> ov, Boolean old_val, Boolean new_val) {
                if(new_val){
                    dataProvGraph.setShowAllNodeIds(true);
                }
                else{
                    dataProvGraph.setShowAllNodeIds(false);
                }
            }
        });

        showRelationships.selectedProperty().addListener(new ChangeListener<Boolean>() {
            public void changed(ObservableValue<? extends Boolean> ov, Boolean old_val, Boolean new_val) {
                if(new_val){
                    dataProvGraph.setShowAllRelationships(true);
                }
                else{
                    dataProvGraph.setShowAllRelationships(false);
                }
            }
        });
    }

    private void initMouseEvents(){
        visCanvas.setOnMouseDragged(new EventHandler<MouseEvent>(){
            @Override
            public void handle(MouseEvent event) {
                if(event.isPrimaryButtonDown()) {
                    if (draggedNode != null) { // drag node
                        draggedNode.setX(event.getX() / zoomRatio + displayWindowLocation[0]);
                        draggedNode.setY(event.getY() / zoomRatio + displayWindowLocation[1]);
                    } else {
                        displayWindowLocation[0] = displayWindowLocationTmp[0] + pressedXY[0] - event.getX() / zoomRatio;
                        displayWindowLocation[1] = displayWindowLocationTmp[1] + pressedXY[1] - event.getY() / zoomRatio;
                    }
                }
            }
        });

        visCanvas.setOnMousePressed(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent event) {
                if(event.isPrimaryButtonDown()) {
                    draggedNode = dataProvGraph.getSelectedNode(event.getX() / zoomRatio + displayWindowLocation[0],
                            event.getY() / zoomRatio + displayWindowLocation[1], zoomRatio);
                    pressedXY = new double[]{event.getX() / zoomRatio, event.getY() / zoomRatio};

                    if (draggedNode == null) {
                        displayWindowLocationTmp = displayWindowLocation.clone();
                    }
                }
            }
        });

        visCanvas.setOnMouseClicked(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent event) {
                if(event.getButton().equals(MouseButton.PRIMARY)) {
                    if(event.getClickCount() == 1) {
                        Edge edge = dataProvGraph.getSelectedEdge(event.getX() / zoomRatio + displayWindowLocation[0],
                                event.getY() / zoomRatio + displayWindowLocation[1], zoomRatio);

                        if (edge != null) {
                            dataProvGraph.addOrRemoveDispRelationship(edge);
                        }
                    }
                }
            }
        });

        visCanvas.setOnMouseMoved(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent event) {
                Node node = dataProvGraph.getSelectedNode(event.getX() / zoomRatio + displayWindowLocation[0],
                        event.getY() / zoomRatio + displayWindowLocation[1], zoomRatio);

                dataProvGraph.setMouseOnNode(node);


                Edge edge = dataProvGraph.getSelectedEdge(event.getX() / zoomRatio + displayWindowLocation[0],
                        event.getY() / zoomRatio + displayWindowLocation[1], zoomRatio);

                dataProvGraph.setMouseOnEdge(edge);
            }
        });

        visCanvas.setOnMouseReleased(new EventHandler<MouseEvent>() {
            @Override
            public void handle(MouseEvent event) {
                if (draggedNode != null && pressedXY[0] == event.getX() / zoomRatio && pressedXY[1] == event.getY() / zoomRatio) {
                    dataProvGraph.addOrRemoveDispNodeId(draggedNode);

                    if(draggedNode instanceof ActivityNode){
                        dataProvGraph.addOrRemoveSelectedActivityNode((ActivityNode)draggedNode);
                    }
                }
                draggedNode = null;
            }
        });

        visCanvas.setOnScroll(new EventHandler<ScrollEvent>() {
            @Override
            public void handle(ScrollEvent event) {
                //update zoomRatio
                double deltaY = event.getDeltaY();
                double oldZoomRatio = zoomRatio;

                if(deltaY > 0){
                    zoomRatio = zoomRatio * (1 + zoomSpeed);
                }
                else if(deltaY < 0){
                    zoomRatio = zoomRatio / (1 + zoomSpeed);
                }

                if(deltaY != 0){
                    displayWindowSize[0] = visCanvas.getWidth() / zoomRatio;
                    displayWindowSize[1] = visCanvas.getHeight() / zoomRatio;
                    displayWindowLocation[0] = ((zoomRatio - oldZoomRatio) / (zoomRatio * oldZoomRatio)) * event.getX() + displayWindowLocation[0];
                    displayWindowLocation[1] = ((zoomRatio - oldZoomRatio) / (zoomRatio * oldZoomRatio)) * event.getY() + displayWindowLocation[1];
                }
            }
        });
    }

    private void initNodeEdge(String provFileURI){
        provModel = RDFDataMgr.loadModel(provFileURI);
        StmtIterator iter = provModel.listStatements();
        NodeFactory nodeFactory = NodeFactory.getInstance();
        EdgeFactory edgeFactory = EdgeFactory.getInstance();
        Statement stmt;

        while (iter.hasNext()) {
            stmt = iter.nextStatement();
            String predicateStr = stmt.getPredicate().toString();
            if(predicateStr.equals(ProvVisGlobal.RDF_TYPE)){
                String subjectStr = stmt.getSubject().toString();
                String objectStr = stmt.getObject().toString();
                if(objectStr.equals(ProvVisGlobal.PROV_ACTIVITY)){
                    if(dataProvGraph.isNodeAdded(subjectStr)) {
                        dataProvGraph.addNode(nodeFactory.convertToActivityNode(dataProvGraph.getNode(subjectStr)));
                    }
                    else {
                        //create Activity Node
                        Node activityNode = nodeFactory.createActivityNode();
                        activityNode.setId(subjectStr)
                                .setX(Math.random() * visCanvas.getWidth())
                                .setY(Math.random() * visCanvas.getHeight());
                        dataProvGraph.addNode(activityNode);
                    }
                }
                else if(objectStr.equals(ProvVisGlobal.PROV_SW_AGENT)){
                    if(dataProvGraph.isNodeAdded(subjectStr)) {
                        dataProvGraph.addNode(nodeFactory.convertToAgentNode(dataProvGraph.getNode(subjectStr)));
                    }
                    else {
                        //create Agent Node
                        Node agentNode = nodeFactory.createAgentNode();
                        agentNode.setId(subjectStr)
                                .setX(Math.random() * visCanvas.getWidth())
                                .setY(Math.random() * visCanvas.getHeight());
                        dataProvGraph.addNode(agentNode);
                    }
                }
                else if(objectStr.equals(ProvVisGlobal.PROV_ENTITY)){
                    if(dataProvGraph.isNodeAdded(subjectStr)) {
                        if(dataProvGraph.getNode(subjectStr).getLabel().equals("commit")){
                            //convert to commit node
                            dataProvGraph.addNode(nodeFactory.convertToCommitNode(dataProvGraph.getNode(subjectStr)));
                        }
                        else {
                            dataProvGraph.addNode(nodeFactory.convertToEntityNode(dataProvGraph.getNode(subjectStr)));
                        }
                    }
                    else {
                        //create Entity Node
                        Node entityNode = nodeFactory.createEntityNode();
                        entityNode.setId(subjectStr)
                                .setX(Math.random() * visCanvas.getWidth())
                                .setY(Math.random() * visCanvas.getHeight());
                        dataProvGraph.addNode(entityNode);
                    }
                }
            }
            else if(predicateStr.equals(ProvVisGlobal.RDF_LABEL)){
                String subjectStr = stmt.getSubject().toString();
                String objectStr = stmt.getObject().toString();

                if(dataProvGraph.isNodeAdded(subjectStr)) {
                    dataProvGraph.getNode(subjectStr).setLabel(objectStr);

                    if(objectStr.equals("commit")){
                        //convert to commit node
                        dataProvGraph.addNode(nodeFactory.convertToCommitNode(dataProvGraph.getNode(subjectStr)));
                    }
                }
                else{
                    //create a Default Node to store the label value.
                    Node node = null;
                    if(objectStr.equals("commit")){
                        node = nodeFactory.createCommitNode();
                    }
                    else {
                        node = nodeFactory.createDefaultNode();
                    }

                    node.setId(subjectStr)
                            .setX(Math.random()*visCanvas.getWidth())
                            .setY(Math.random()*visCanvas.getHeight())
                            .setLabel(objectStr);
                    dataProvGraph.addNode(node);
                }
            }
            else if(stmt.getObject().isURIResource()){
                //Skip "wasGeneratedBY" edge to avoid duplicate relationship display temporary, will find out a better
                //way to disply two or more relationship later
                if(stmt.getPredicate().toString().equals(ProvVisGlobal.PROV_WAS_GENERATED_BY)){
                    continue;
                }

                //create a Default Node to store the label value.
                Edge defaultEdge = edgeFactory.createDefaultEdge();
                defaultEdge.setFromNodeId(stmt.getSubject().toString())
                        .setToNodeId(stmt.getObject().toString())
                        .setRelationship(stmt.getPredicate().toString());
                dataProvGraph.addEdge(defaultEdge);
            }
            //System.out.println(stmt.getSubject().toString() + " " + stmt.getPredicate().toString() + " " + stmt.getObject().toString());
        }

        //set neighbors
        dataProvGraph.setNeighbors();
    }
}
