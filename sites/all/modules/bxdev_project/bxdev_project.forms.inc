<?php

/**
 * FORMS
 */

/**
 * Implements hook_form_FORM_ID_alter().
 */
function bxdev_project_form_project_node_form_alter(&$form, $form_state){
	
	global $user;
	// set the author value, if sales/add/project
	if(arg(0) == 'sales'){
		$form['field_author']['und']['#type'] = 'hidden';
		// only pre-populate if sales user
		if(bxdev_user_get_role($user->uid) == 'sales'){
			$form['field_author']['und']['#default_value'] = $user->uid;		
		}else if(bxdev_user_get_role($user->uid) == 'pc'){
			$form['field_author']['und']['#default_value'] = '_none';
		}
	}

	// remove the N/A options from video type, voice over, voice over gender, and owner interview
	unset($form['field_video_type']['und']['#options']['_none']);
	unset($form['field_voice_over']['und']['#options']['_none']);
	unset($form['field_voice_over_gender']['und']['#options']['_none']);
	unset($form['field_owner_interview']['und']['#options']['_none']);
		
	// add custom validation handler
	$form['actions']['submit']['#validate'][] = 'bxdev_project_form_project_node_form_validate';
	
	$times = bxdev_get_times_array();
	
	// create a fieldset
	$form['bxdev_shoot_date_fieldset'] = array(
		'#type' => 'fieldset',
		'#weight' => 13,
		'#attributes' => array(
			'class' => array('clearfix'),
		),
	);
	// create a custom date field
	$form['bxdev_shoot_date_fieldset']['bxdev_shoot_date'] = array(
		'#type' => 'textfield',
		'#title' => t('Shoot date'),
		'#required' => TRUE,
		'#default_value' => (!empty($form['field_shoot_date']['und'][0]['#default_value']['value'])) ? date('m/d/Y', $form['field_shoot_date']['und'][0]['#default_value']['value']) : '',
	);
	// create a custom start time field
	$form['bxdev_shoot_date_fieldset']['bxdev_shoot_time1'] = array(
		'#title' => t('Start time'),
		'#type' => 'select',
		'#required' => TRUE,
		'#options' => $times,
		'#default_value' => (!empty($form['field_shoot_date']['und'][0]['#default_value']['value'])) ? date('G:i', $form['field_shoot_date']['und'][0]['#default_value']['value']) : '',
	);
	// create a custom end time field
	$form['bxdev_shoot_date_fieldset']['bxdev_shoot_time2'] = array(
		'#title' => t('End time'),
		'#type' => 'select',
		'#options' => $times,
		'#default_value' => ($form['field_shoot_date']['und'][0]['#default_value']['value2'] != $form['field_shoot_date']['und'][0]['#default_value']['value']) ? date('G:i', $form['field_shoot_date']['und'][0]['#default_value']['value2']) : '',
	);
	
	// do not modify on admin side
	if(arg(0) != 'node' && !is_numeric(arg(1))){
		
		// set the user reference uid
		$nid = arg(3);
		$form['field_project_business_ref']['und']['#default_value'] = $nid;
	
		// override original submit button
		$form['actions']['submit']['#value'] = t('Submit & View Profile');
	
		// add additional submit button
		$form['actions']['return'] = array(
			'#type' => 'submit',
			'#value' => t('Submit & Return to Form'),
			'#submit' => array('node_form_submit', 'bxdev_project_project_node_form_submit'),
			'#validate' => array('bxdev_project_form_project_node_form_validate'),
		);
		
		$form['field_project_business_ref']['#prefix'] = '<div class="left">';
		$form['field_owner_interview']['#suffix'] = '</div><div class="right">';
		$form['actions']['#prefix'] = '</div>';		
	}
		
}

/**
 * Form submit handler: add project
 */
function bxdev_project_project_node_form_submit($form, &$form_state){
		
	// send to client add form if "return to form" button was clicked
	if($form_state['clicked_button']['#value'] == 'Submit & Return to Form'){
		$form_state['redirect'] = 'sales/add/business';
	}
}

/**
 * Form submit handler: project node form
 */
function bxdev_project_form_project_node_form_validate($form, &$form_state){
	// get all the values
	$values = $form_state['values'];
	
	// video type cannot be left blank
	if(!$values['field_video_type']['und'][0]['value']){
		form_set_error('field_video_type[und]', 'A video type must be selected');
	}
	
	// voice over cannot be left blank
	if(!$values['field_voice_over']['und'][0]['value']){
		form_set_error('field_voice_over[und]', 'Voice over option must be selected');
	}
	
	// voice over gender cannot be left blank
	if(!$values['field_voice_over_gender']['und'][0]['value']){
		form_set_error('field_voice_over_gender[und]', 'A voice over gender must be selected');
	}
	
	// owner interview cannot be left blank
	if(!$values['field_owner_interview']['und'][0]['value']){
		form_set_error('field_owner_interview[und]', 'Business owner interview must be selected');
	}
	
	// check if end time is less than start time
	if($values['bxdev_shoot_time2'] != ''){
		if($values['bxdev_shoot_time2'] < $values['bxdev_shoot_time1']){
			form_set_error('bxdev_shoot_time2', 'End time cannot be less than start time');
		}
		
	}
	
}

/**
 * Form builder: photo approval
 */
function bxdev_project_photo_approval_form($form, $form_state){
	$form = array();
	$node = node_load(arg(1));
	$render = node_view($node, 'teaser');
	$full = node_view($node, 'full');
	
	// if photos have been assigned
	if(!empty($render['field_project_photos'])){
		
		// get stored values
		$results = db_query("SELECT fid FROM {project_photos_approval} WHERE nid = :nid", array(':nid' => $node->nid));
		$defaults = array();
		if($results->rowCount() > 0){
			foreach($results as $row){
				$defaults[] = $row->fid;
			}
		}
		
		// build the images and checkboxes
		foreach($render['field_project_photos'] as $key => $value){
			if(is_numeric($key)){
				$path = file_create_url($full['field_project_photos'][$key]['#item']['uri']);
				$form['approve'][$value['#item']['fid']] = array(
				  '#type' => 'checkbox',
				  '#title' => t('Approve'),
				  '#default_value' => in_array($value['#item']['fid'], $defaults),
					'#prefix' => '<div class="photo-wrap"><a rel="shadowbox[images]" href="' . image_style_url('approval', $full['field_project_photos'][$key]['#item']['uri']) . '">' . theme($value['#theme'], $value) . '</a>',
					'#suffix' => '</div>',
				);				
			}
		}
		
		// submit button
		$form['submit'] = array(
		  '#type' => 'submit',
		  '#value' => t('Submit'),
		);		
		
	// photos have not been assigned
	}else{
		$form['no_photos'] = array(
			'#markup' => t('No photos assigned.'),
		);
	}
	
	return $form;
}

/**
 * Form submit handler: photo approval form
 */
function bxdev_project_photo_approval_form_submit($form, &$form_state){
	$nid = arg(1);
	$values = $form_state['values'];
	// clear all values for current node
	db_delete('project_photos_approval')
		->condition('nid', $nid)
		->execute();
	// store each new value
	foreach($values as $key => $value){
		if(is_numeric($key)){
			if($value){
				db_insert('project_photos_approval')
					->fields(array(
						'nid' => $nid,
						'fid' => $key,
					))
					->execute();
			}
		}
	}
	drupal_set_message('Photo approvals have been updated.');
}

/**
 * Form validation handler: photo approval form
 */
function bxdev_project_photo_approval_form_validate($form, &$form_state){
	$values = $form_state['values'];
	$count = 0;
	// cycle through each photo and allow no more than eight choices
	foreach($values as $key => $value){
		if(is_numeric($key)){
			$count += $value;
		}
	}
	// check if greater than 8
	if($count > 8){
		form_error($form, 'A maximum of 8 choices can be approved.');
		return FALSE;
	}
}

/**
 * Form builder: business logo
 */
function bxdev_project_logo_upload_form($form, &$form_state){
	$nid = arg(1);
	$node = node_load($nid);
	$count = 0;
	
	$form['#tree'] = TRUE;
	
	$form['title']['#markup'] = t('<label>Business logos</label>');
	$form['title']['#prefix'] = '<div class="logo-upload-wrapper">';
	
	if(!empty($node->field_logo['und'])){
		foreach($node->field_logo['und'] as $key => $fid){
			$form['business_logo'][$key] = array(
				'#type' => 'managed_file',
				'#size' => 30,
				'#default_value' => $fid['fid'],
				'#upload_location' => 'public://business_logos',
				'#upload_validators' => array(
					'file_validate_extensions' => array('jpg jpeg gif png pdf'),
				),
			);
			$count++;	
		}
	}
	
	$form['business_logo'][$count] = array(
		'#type' => 'managed_file',
		'#size' => 30,
		'#upload_location' => 'public://business_logos',
		'#upload_validators' => array(
			'file_validate_extensions' => array('jpg jpeg gif png pdf'),
		),
	);
	
	$form['save'] = array(
	  '#type' => 'submit',
	  '#value' => t('Save'),
		'#suffix' => '</div>',
	);	
	
	return $form;
}

/**
 * Form submit handler: business logo
 */
function bxdev_project_logo_upload_form_submit($form, &$form_state){
	$values = $form_state['values'];
	$nid = arg(1);
	$node = node_load($nid);
	unset($node->field_logo['und']);
	foreach($values['business_logo'] as $key => $fid){
		if($fid !== 0){
			// get the file object
			$file = file_load($fid);
			$file->status = FILE_STATUS_PERMANENT;
			file_save($file);
			// create a file array to attach to the node
			$node_file = array(
				'uid' => $file->uid,
				'uri' => $file->uri,
				'fid' => $file->fid,
				'status' => 1,
				'display' => 1,
				'description' => '',
			);
			$node->field_logo['und'][$key] = $node_file;	
		}		
	}
	node_save($node);
}

/**
 * Form builder: videographer documents
 */
function bxdev_project_videographer_documents_form($form, &$form_state){
	$nid = arg(1);
	$node = node_load($nid);
	
	$form['signed_production_release'] = array(
		'#type' => 'managed_file',
		'#title' => t('Signed production release'),
		'#size' => 15,
		'#default_value' => (!empty($node->field_signed_production_release['und'][0])) ? $node->field_signed_production_release['und'][0]['fid'] : '',
		'#upload_location' => 'public://videographer_forms',
		'#upload_validators' => array(
			'file_validate_extensions' => array('jpg jpeg gif png pdf doc docx'),
		),
	);
	
	$form['videographer_invoice'] = array(
		'#type' => 'managed_file',
		'#title' => t('Videographer invoice'),
		'#size' => 15,
		'#default_value' => (!empty($node->field_videographer_invoice['und'][0])) ? $node->field_videographer_invoice['und'][0]['fid'] : '',
		'#upload_location' => 'public://videographer_forms',
		'#upload_validators' => array(
			'file_validate_extensions' => array('jpg jpeg gif png pdf doc docx'),
		),
	);
	
	$form['save'] = array(
	  '#type' => 'submit',
	  '#value' => t('Save'),
	);	
	
	return $form;
}

/**
 * Form submit handler: videographer documents
 */
function bxdev_project_videographer_documents_form_submit($form, &$form_state){
	$values = $form_state['values'];
	$nid = arg(1);
	$node = node_load($nid);
		
	// if no file, remove logo file from the node
	if(empty($values['signed_production_release'])){
		unset($node->field_signed_production_release['und'][0]);
	}else{
		// get the file object
		$file = file_load($values['signed_production_release']);
		$file->status = FILE_STATUS_PERMANENT;
		file_save($file);
		// create a file array to attach to the node
		$node_file = array(
			'uid' => $file->uid,
			'uri' => $file->uri,
			'fid' => $file->fid,
			'status' => 1,
			'display' => 1,
			'description' => '',
		);
		$node->field_signed_production_release['und'][0] = $node_file;
	}
	
	// if no file, remove logo file from the node
	if(empty($values['videographer_invoice'])){
		unset($node->field_videographer_invoice['und'][0]);
	}else{
		// get the file object
		$file = file_load($values['videographer_invoice']);
		$file->status = FILE_STATUS_PERMANENT;
		file_save($file);
		// create a file array to attach to the node
		$node_file = array(
			'uid' => $file->uid,
			'uri' => $file->uri,
			'fid' => $file->fid,
			'status' => 1,
			'display' => 1,
			'description' => '',
		);
		$node->field_videographer_invoice['und'][0] = $node_file;
	}
	
	node_save($node);
	
}

