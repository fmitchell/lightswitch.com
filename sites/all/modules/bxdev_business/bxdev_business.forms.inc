<?php

/**
 * Implements hook_form_FORM_ID_alter().
 * Form: business node form
 */
function bxdev_business_form_business_node_form_alter(&$form, $form_state){

	$form['#attributes']['class'][] = 'clearfix';
	$form['actions']['submit']['#submit'][] = 'bxdev_business_form_business_node_form_submit';
	$form['#validate'][] = 'bxdev_business_form_business_node_form_validate';
	$form['actions']['submit']['#value'] = t('Continue');
	$form['field_social_name3']['und'][0]['value']['#value'] = 'Yelp';

	// if sales/add/business
	if(arg(2) !== 'edit'){
		$form['title']['#prefix'] = '<div class="left">';
		$form['field_hours_operation']['#suffix'] = '</div><div class="right">';
		$form['field_business_download_file']['#suffix'] = '</div>';
	}

	// remove labels and set input values for social media inputs
	$form['field_social_name1']['und'][0]['value']['#title'] = t('Social Name');
	$form['field_social_name2']['und'][0]['value']['#title'] = '';
	$form['field_social_name3']['und'][0]['value']['#title'] = '';
	$form['field_social_name4']['und'][0]['value']['#title'] = '';
	$form['field_social_name1']['und'][0]['value']['#value'] = t('Facebook');
	$form['field_social_name2']['und'][0]['value']['#value'] = t('Twitter');
	$form['field_social_url1']['und'][0]['value']['#title'] =  t('Social URL');
	$form['field_social_url2']['und'][0]['value']['#title'] =  '';
	$form['field_social_url3']['und'][0]['value']['#title'] =  '';
	$form['field_social_url4']['und'][0]['value']['#title'] =  '';

	// if arg 3 is set, that means we have added a new client / credit card in a previous step
	$arg3 = arg(3);

	if(!empty($arg3)){

		$account = user_load($arg3);

		$form['field_client_ref']['und']['#type'] = 'hidden';
		$form['field_client_ref']['und']['#value'] = $arg3;
		$form['field_client_ref']['und']['#prefix'] = t('<h3><strong>Client: @name</strong></h2>', array('@name' => $account->field_name['und'][0]['value']));

	// if arg 3 is not set, that means we have not added a new client / credit card in a previous step
	}else if(arg(2) != 'edit'){

		$form['field_client_ref']['und']['#description'] = t('<a href="" class="new-client-btn">+ Add new Client</a>');
		$form['field_client_ref']['#attributes']['style'][] = 'display:none;';

		$form['new_client_email'] = array(
			'#type' => 'textfield',
			'#title' => t('Client email'),
			'#weight' => -50,
			'#prefix' => '<div class="new-client-wrapper">',
		);

		$form['new_client_name'] = array(
			'#type' => 'textfield',
			'#title' => t('Client full name'),
			'#weight' => -49,
		);

		$form['new_client_phone'] = array(
			'#type' => 'textfield',
			'#title' => t('Client phone'),
			'#weight' => -48,
			'#suffix' => '</div>',
			'#description' => t('<a href="" class="new-client-btn">Existing Client</a>'),
		);

	}

	// dpm($form_state);

}

/**
 * Form submit handler: add business
 */
function bxdev_business_form_business_node_form_submit($form, &$form_state){

		$values = $form_state['values'];

		// if a new client email is supplied
		if($values['new_client_email'] != ''){
			// create a new client user
			$user = new User();
			$user->set_email($values['new_client_email']);
			$user->set_role('client');
			$user->set_full_name($values['new_client_name']);
			$user->set_phone($values['new_client_phone']);
			$user->create();
			$user = $user->get_user();

			// load the business node that was just submitted
			$node = node_load($values['nid']);
			// set the client value
			$node->field_client_ref['und'][0]['uid'] = $user->uid;
			node_save($node);
		}

		// only if adding new business, not editing
		if(arg(2) != 'edit'){
			$form_state['redirect'] = 'sales/add/project/' . $values['nid'];
		}

}

/**
 * Form validation handler: add business
 */
function bxdev_business_form_business_node_form_validate($form, &$form_state){
	$values = $form_state['values'];

	// if neither an existing or new client has been entered
	if(is_null($values['field_client_ref']['und'][0]['uid']) && empty($values['new_client_email'])){
		form_set_error('field_client_ref[und]', 'You must select an existing Client or add a new one');

	// if a new client email has been entered
	}else if(!empty($values['new_client_email'])){

		// if client email is not a valid email address
		if(!valid_email_address($values['new_client_email'])){
			form_set_error('new_client_email', 'Enter a valid Client email address');
		}

		// if client email already exists
		if(user_load_by_name($values['new_client_email'])){
			form_set_error('new_client_email', 'Client user ' . $values['new_client_email'] . ' already exists');
		}

		// if client full name was not entered
		if(empty($values['new_client_name'])){
			form_set_error('new_client_name', 'When adding a new Client, the full name must be entered');
		}

		// if client phone was not entered
		if(empty($values['new_client_phone'])){
			form_set_error('new_client_phone', 'When adding a new Client, the phone must be entered');
		}

	}
}

/**
 * Form builder: existing client
 */
function bxdev_business_existing_business_form($form, $form_state){

	// check if a client uid exists
	$arg3 = arg(3);
	$uri = $_SERVER['REQUEST_URI'];

	$form['#action'] = $uri . '#existing';

	$form['existing_business'] = array(
		'#type' => 'textfield',
		'#title' => t('Existing business'),
		'#autocomplete_path' => !empty($arg3) ? 'sales/autocomplete/businesses/' . $arg3 : 'sales/autocomplete/businesses',
		'#required' => TRUE,
	);

	$form['existing_business_nid'] = array(
	  '#type' => 'hidden',
		'#title' => t('A vaild existing business'),
		'#required' => TRUE,
	  '#default_value' => '',
	);

	$form['submit'] = array(
	  '#type' => 'submit',
	  '#value' => t('Continue'),
	);

	return $form;
}

/**
 * Form submit handler: existing client
 */
function bxdev_business_existing_business_form_submit($form, &$form_state){
	$values = $form_state['values'];
	// redirect to the add project page
	$form_state['redirect'] = 'sales/add/project/' . $values['existing_business_nid'];
}

/**
 * New business form
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 * @return unknown
 */
function bxdev_business_new_business_form($form, &$form_state) {
  //mask
  drupal_add_js(drupal_get_path('module', 'bxdev_business') . '/bxdev_business.js');
  drupal_add_js(drupal_get_path('theme', 'bms') . '/js/chosen/chosen.jquery.min.js');
  drupal_add_css(drupal_get_path('theme', 'bms') . '/js/chosen/chosen.css');

  $form = array();

  $form['client'] = array(
    '#type' => 'fieldset',
    '#title' => t('Client'),
    '#tree' => TRUE,
    '#attributes' => array(
			'class' => array('clearfix'),
		),
  );
  $form['client']['client_type'] = array(
    '#type' => 'radios',
    '#title' => '',
    '#options' => array('new' => 'New Client', 'existing' => 'Existing Client'),
    '#default_value' => 'new',
    '#prefix' => '<div class="client-select">',
    '#suffix' => '</div>',
  );
  $form['client']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ':input[name="client[client_type]"]' => array('value' => 'new'),
      ),
    ),
  );
  $form['client']['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ':input[name="client[client_type]"]' => array('value' => 'new'),
      ),
    ),
  );
  $form['client']['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#attributes' => array(
      'class' => array('form-phone'),
    ),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
    '#states' => array(
      'visible' => array(
        ':input[name="client[client_type]"]' => array('value' => 'new'),
      ),
    ),
  );
  $form['client']['existing'] = array(
    '#type' => 'select',
    '#title' => t('Client'),
    '#options' => bxdev_business_clients(),
    '#states' => array(
      'visible' => array(
        ':input[name="client[client_type]"]' => array('value' => 'existing'),
      ),
    ),
    '#attributes' => array(
      'class' => array('form-existing-clients'),
    ),
  );

  $form['business'] = array(
    '#type' => 'fieldset',
    '#title' => t('Business'),
    '#tree' => TRUE,
    '#attributes' => array(
			'class' => array('clearfix'),
		),
  );
  $form['business']['name'] = array(
    '#type' => 'textfield',
    '#title' => t('Name'),
    '#required' => TRUE,
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['business']['address'] = array(
    '#type' => 'textfield',
    '#title' => t('Address'),
    '#required' => TRUE,
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['business']['phone'] = array(
    '#type' => 'textfield',
    '#title' => t('Phone'),
    '#required' => TRUE,
    '#attributes' => array(
      'class' => array('form-phone'),
    ),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['business']['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
    '#required' => TRUE,
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['business']['city_state'] = array(
    '#type' => 'select',
    '#title' => t('City, State'),
    '#required' => TRUE,
    '#options' => bxdev_get_city_array(),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['business']['video_approval'] = array(
    '#type' => 'select',
    '#title' => t('Who approves video?'),
    '#required' => TRUE,
    '#options' => array('client' => 'Client', 'business' => 'Business'),
    '#prefix' => '<div class="clearfix"></div>',
  );

  $form['production'] = array(
    '#type' => 'fieldset',
    '#title' => t('Production Details'),
    '#tree' => TRUE,
    '#attributes' => array(
			'class' => array('clearfix'),
		),
  );
  $form['production']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
    '#required' => TRUE,
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['production']['sale_type'] = array(
    '#type' => 'select',
    '#title' => t('Sale Type'),
    '#required' => TRUE,
    '#options' => array('spec' => 'Spec', 'direct_sale' => 'Direct sale', 'bd' => 'Business dev'),
  );
  $form['production']['csa'] = array(
    '#type' => 'checkbox',
    '#title' => t('Do not send Client Service Agreement'),
    '#states' => array(
      'visible' => array(
        ':input[name="production[sale_type]"]' => array('value' => 'direct_sale'),
      ),
    ),
  );
  $form['production']['shoot_date_start'] = array(
    '#type' => 'textfield',
    '#title' => t('Start Date & Time'),
    '#attributes' => array(
      'class' => array('form-date'),
    ),
    '#prefix' => '<div class="clearfix"></div><div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['production']['shoot_date_end'] = array(
    '#type' => 'textfield',
    '#title' => t('End Date & Time'),
    '#attributes' => array(
      'class' => array('form-date'),
    ),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['production']['project_type'] = array(
    '#type' => 'select',
    '#title' => t('Project Type'),
    '#required' => TRUE,
    '#options' => array('449' => 'Standard video', '549' => 'Video with interview', '599' => 'Video with voice over', '50' => 'Monthly Subscription', 'custom' => 'Custom Services'),
    '#prefix' => '<div class="clearfix"></div><div class="form-float-left">',
    '#suffix' => '</div>',
  );
  $form['production']['client_price'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Price'),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
    '#attributes' => array(
      'class' => array('form-price'),
    ),
  );
  $form['production']['client_notes'] = array(
    '#type' => 'textarea',
    '#title' => t('Client Notes'),
    '#prefix' => '<div class="form-float-left">',
    '#suffix' => '</div>',
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Continue'),
    '#attributes' => array(
      'class' => array('continue'),
    ),
  );

  return $form;
}

/**
 * New business form submit handler
 *
 * @param unknown_type $form
 * @param unknown_type $form_state
 */
function bxdev_business_new_business_form_submit($form, &$form_state) {
  $values = $form_state['values'];

  //save client
  if ($values['client']['client_type'] == 'new') {
    $user = new User();
  	$user->set_email($values['client']['email']);
  	$user->set_role('client');
  	$user->set_full_name($values['client']['name']);
  	$user->set_phone($values['client']['phone']);
  	$user->create();
  	$user = $user->get_user();
  }
  else {
    $user = user_load($values['client']['existing']);
  }


	//save business
	$node = new stdClass();
	$node->type = 'business';
	node_object_prepare($node);
	$node->field_client_ref[LANGUAGE_NONE][0]['uid'] = $user->uid;
	$node->title = $values['business']['name'];
	$node->field_address[LANGUAGE_NONE][0]['value'] = $values['business']['address'];
	$node->field_phone[LANGUAGE_NONE][0]['value'] = $values['business']['phone'];
	$node->field_email[LANGUAGE_NONE][0]['value'] = $values['business']['email'];
	$node->field_business_city[LANGUAGE_NONE][0]['value'] = $values['business']['city_state'];
	$node->field_business_video_approve[LANGUAGE_NONE][0]['value'] = $values['business']['video_approval'];
	node_save($node);
	$business = $node;

	//save project
	$node = new stdClass();
	$node->type = 'project';
	node_object_prepare($node);
	$node->title = $values['production']['title'];
	$node->field_project_business_ref[LANGUAGE_NONE][0]['nid'] = $business->nid;
	$node->field_video_type[LANGUAGE_NONE][0]['value'] = $values['production']['sale_type'];
	$node->field_client_send_terms[LANGUAGE_NONE][0]['value'] = $values['production']['csa'];

	$node->field_shoot_date[LANGUAGE_NONE][0]['value'] = strtotime($values['production']['shoot_date_start']);
	$node->field_shoot_date[LANGUAGE_NONE][0]['value2'] = strtotime($values['production']['shoot_date_end']);
	$node->field_shoot_date[LANGUAGE_NONE][0] += array('date_type' => 'datestamp', 'timezone' => 'America/Chicago', 'timezone_db' => 'UTC');

  $node->field_project_type[LANGUAGE_NONE][0]['value'] = $values['production']['project_type'];
  $node->field_charge_amount[LANGUAGE_NONE][0]['value'] = $values['production']['client_price'];
  $node->field_pc_notes[LANGUAGE_NONE][0]['value'] = $values['production']['client_notes'];
  node_save($node);

  $form_state['redirect'] = 'user';
}








